#!/usr/bin/env ruby
# -*- coding: utf-8 -*-
require "pathname"
require "fileutils"
require "coderay"
require "kramdown"

# Sourcecode parsing class that extracts scripting-related
# comments from all sourcefiles/directories passed to ::new.
class Parser

  # Represents a documented class.
  ClassDoc = Struct.new(:name, :documentation)
  # Represents a documented module.
  ModuleDoc = Struct.new(:name, :documentation)
  # Represents a documented method.
  MethodDoc = Struct.new(:name, :classname, :is_instance_method, :call_seqs, :documentation)

  # Turn on debug output?
  attr_accessor :debug
  # The list of sources passed to ::new, as absolute Pathname
  # instances.
  attr_reader :sources
  # After the call to #parse! a sorted array of ClassDoc instances.
  attr_reader :classes
  # After the call to #parse! a sorted array of ModuleDoc instances.
  attr_reader :modules
  # After the call to #parse! a sorted array of MethodDoc instances.
  attr_reader :methods

  # Instruct the parser to parse the given files. Directories
  # are traversed recursively looking for all files ending
  # in c,cpp,h,hpp.
  def initialize(*sources)
    @sources = sources.map{|s| Pathname.new(s).expand_path}
    @debug   = false
    @classes = []
    @modules = []
    @methods = []
    @document_block_open = false
    @doctext = ""
  end

  # Starts the parsing process.
  def parse!
    @sources.each do |source|
      if source.directory?
        source.find do |path|
          next unless path.to_s =~ /\.(?:c|cpp|h|hpp)$/
          parse_file(path)
        end
      elsif source.file?
        parse_file(source)
      else
        warn "Warning: Cannot read '#{source}'. Ignoring."
      end
    end

    @classes.sort_by!(&:name)
    @modules.sort_by!(&:name)
    @methods.sort_by!(&:name)

    print_summary
  end

  private

  # Print +str+ in debug mode, no-op otherwise.
  def debug(str)
    puts(str) if @debug
  end

  def parse_file(path)
    puts "Examining #{path.relative_path_from(Pathname.pwd)}..."
    path.open do |file|
      file.each_line.with_index do |line, lino|

        if @document_block_open
          # We are in a /** block here
          if line =~ /^\s*\*\/$/
            # /** block closed by */
            debug "> Closing document block on line #{@lino}"
            @document_block_open = false

            parse_doctext
            @doctext.clear
          else
            # Inside open /** block
            text = line.strip
            text.replace($') if text =~ /^\*\s?/
            @doctext << text << "\n"
          end
        else
          # We are not in a /** block here
          if line =~ /^\/\*\*$/
            # /** block opened
            @lino = lino + 1
            debug "> Opening document block on line #{@lino}"
            @document_block_open = true
          end
        end
      end
    end
  end

  def parse_doctext
    if @doctext.lines.first =~ /^([a-zA-Z]+):/
      method = "parse_doctype_#{$1.downcase}"

      # If we know how to parse the requested documentation type,
      # do so. Otherwise print a warning and ignore this block.
      if respond_to?(method, true)
        send(method, $'.strip)
      else
        warn "Warning: Skipping invalid documentation type '#{$1}' on line #@lino"
      end
    else
      warn "Warning: Skipping invalid documentation comment block on line #@lino"
    end
  end

  # Parse a class documentation block, which must be of the following
  # form:
  #
  #   /**
  #    * Class: MyClassName
  #    *
  #    * Class documentation goes here, probably
  #    * multiline.
  #    */
  def parse_doctype_class(classname)
    debug "> Detected class '#{classname}'"

    @classes << ClassDoc.new(classname, @doctext.lines.drop(1).join)
  end

  # Parse a module documentation block, which must be of the following
  # form:
  #
  #   /**
  #    * Module: MyModuleName
  #    *
  #    * Module documentation goes here, probably
  #    * multiline.
  #    */
  def parse_doctype_module(modname)
    debug "> Detected module '#{modname}'"

    @modules << ModuleDoc.new(modname, @doctext.lines.drop(1).join)
  end

  # Prase a method documentation block, which must be of the following
  # form (use :: instead of # for class methods):
  #
  #   /**
  #    * Method: MyClassName#my_method_name
  #    *
  #    *   my_method_name(my_arg) → my_retval
  #    *   my_method_name{|blockarg| ... } → my_retval
  #    *
  #    * Documentation goes here, probably
  #    * multiline.
  #    */
  def parse_doctype_method(methodstring)
    debug "> Detected method: '#{methodstring}'"

    # Check whether class or instance methods (or invalid)
    if methodstring.include?("#")
      classname, methodname = methodstring.split("#")
      is_imethod = true
    elsif methodstring.include?("::")
      parts      = methodstring.split("::")
      methodname = parts.pop
      classname  = parts.join("::")
      is_imethod = false
    else
      warn "Warning: Invalid method spec '#{methodstring}' on line #@lino. Ignoring."
      return
    end

    # Divide the rest of the documentation block into the call sequences
    # and the real documentation.
    calls = []
    text = @doctext.lines.drop(1).drop_while do |line|
      next true if line.strip.empty? # Ignore newlines surrounding the call sequences block

      if line =~ /^\s/
        calls << line.strip
        true
      else
        false
      end
    end

    warn "Warning: No call sequence found for '#{methodstring}' on line #@lino" if calls.empty?

    @methods << MethodDoc.new(methodname, classname, is_imethod, calls, text.join)
  end

  # Output a nice summary of what we found.
  def print_summary
    puts
    puts
    puts "=== SUMMARY ==="
    puts "Classes: #{@classes.count}"
    puts "Modules: #{@modules.count}"
    puts "Methods: #{@methods.count}"
    puts
    puts
  end

end

class KramdownGenerator

  def initialize(targetdir, templatefile, indexfile, classes, modules, methods)
    @targetdir    = Pathname.new(targetdir).expand_path
    @templatefile = Pathname.new(templatefile).expand_path
    @indexfile    = Pathname.new(indexfile).expand_path
    @classes      = classes
    @modules      = modules
    @methods      = methods
  end

  def generate!
    # Generate the HTML files for all the classes
    puts "Generating classes..."
    @classes.each do |klass|
      generate_class(klass)
    end

    # Go for the modules
    puts "Generating modules..."
    @modules.each do |mod|
      generate_module(mod)
    end

    # Last but not least convert the index file
    puts "Generating index.html..."
    @targetdir.join("index.html").open("w") do |file|
      file.write(kramdown(@indexfile.read))
    end
  end

  private

  [:class, :module].each do |sym|
    define_method(:"generate_#{sym}") do |klassmod|
      result = ""

      # Site header
      result << "#{sym.to_s.capitalize} " << klassmod.name << "\n"
      result << "=" * (result.chars.count - 1) << "\n"

      # Advise kramdown to create a ToC
      result << "\n* This is the\n{:toc}\n\n"

      # Add the real class docs
      result << klassmod.documentation

      # Find the methods we need for us
      methods  = @methods.select{|m| m.classname == klassmod.name}
      imethods = methods.select(&:is_instance_method)
      cmethods = methods.reject(&:is_instance_method)

      # Now for the class methods
      unless cmethods.empty?
        result << "\n\n#{sym.to_s.capitalize} methods\n"
        result << "-" * "#{sym.to_s.capitalize} methods".chars.count << "\n\n"

        cmethods.each{|cm| result << generate_method(cm) << "\n"}
      end

      # Same for the instance methods
      unless imethods.empty?
        result << "\n\nInstance methods\n"
        result << "-" * "Instance methods".chars.count << "\n\n"

        imethods.each{|im| result << generate_method(im) << "\n"}
      end

      # Let kramdown transform it to HTML
      @targetdir.join("#{klassmod.name.downcase.gsub("::", "_")}.html").open("w") do |file|
        file.write(kramdown(result))
      end
    end
  end

  def generate_method(method)
    result = ""

    ### method_name ###
    result << "### #{method.name} ###" << "\n"

    # Call sequences
    method.call_seqs.each do |cq|
      result << "    " << cq
    end
    result << "\n\n"

    # Real documentation
    result << method.documentation << "\n"

    # Done
    result
  end

  # Takes markdown string and returns the corresponding HTML.
  def kramdown(str)
    Kramdown::Document.new(str,
                           toc_levels: "2..3",
                           template: @templatefile.to_s,
                           enable_coderay: true,
                           coderay_line_numbers: :table,
                           coderay_css: :class).to_html
  end

end

if $0 == __FILE__
  include FileUtils

  this_dir      = Pathname.new(__FILE__).dirname.expand_path
  template_file = this_dir + "template.html.erb"
  index_file    = this_dir + "index.md"
  target_dir    = this_dir + "html"
  source_dir    = this_dir + ".." + ".." + "src" + "scripting"

  print "Creating #{target_dir}/... directories... "
  rm_rf target_dir
  mkdir_p target_dir + "graphics"
  puts "Done."

  print "Copying graphics... "
  cp this_dir + "../../data/pixmaps/game/items/cookie.png", target_dir + "graphics"
  cp this_dir + "../../data/pixmaps/enemy/furball/brown/turn.png", target_dir + "graphics/furball.png"
  cp this_dir + "../../data/pixmaps/enemy/eato/brown/3.png", target_dir + "graphics/eato.png"
  cp this_dir + "../../data/pixmaps/enemy/flyon/orange/open_1.png", target_dir + "graphics/flyon.png"
  cp this_dir + "../../data/pixmaps/enemy/gee/electro/5.png", target_dir + "graphics/gee.png"
  puts "Done."

  print "Copying stylesheets... "
  cp this_dir + "coderay.css", target_dir
  cp this_dir + "style.css", target_dir
  puts "Done."

  puts "Parsing scripting source files."
  ARGV.unshift(source_dir) if ARGV.empty?
  parser = Parser.new(*ARGV)
  # parser.debug = true
  parser.parse!
  gen = KramdownGenerator.new(target_dir, template_file, index_file, parser.classes, parser.modules, parser.methods)
  gen.generate!

  puts "Finished."
end
