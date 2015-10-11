Maintenance of the ChangeLog
============================

The purpose of the CHANGELOG file is to inform users of TSC about
relevant changes between versions, i.e. it is not inded to provide an
in-depth introduction into the how and why of changes; this
information belongs to the Git revision history. It follows that the
CHANGELOG should contain those changes that are somehow relevant for
the players, the packagers, and other users of the software, but not
information only related to code changes.

Each nontrivial change that affects the outer experience of the game
shall be recorded in the file named CHANGELOG in the toplevel of the
repository. The format of that file is a list of entries that look
like this:

~~~~~~~~~~~~~~~~~~~~~~~
date  Releasing-Person  <email-of-that-person>
 * Version number
 * Change 1
 * Change 2
 * Change 3
 * ...
~~~~~~~~~~~~~~~~~~~~~~~

“date” is to be replaced with the release date of the version,
“Releasing-Person” is to be replaced with the person doing the final
release, i.e. the one who tags the release in the repository and
uploads the file to the server. “email-of-that-person” is replaced by
his email address.

The changelog entry starts with the literal word “Version” followed by
version number of the version that is released. As during development
it is usually not clear what the next version is, it is okay to have
that line look like

~~~~~~~~~~~~~~~~~~~~~~~
  * Version unreleased.
~~~~~~~~~~~~~~~~~~~~~~~

until it becomes clear, which version will be the next one and then
change it accordingly.

Below that, each change is recorded starting with a bullet
(asterisk). Each of those changes starts with one of these words:

* BREAKING: A change breaking outer compatbility was introduced.
* Feature: A new feature was added.
* Fix: Some bug was fixed.
* Misc: Things that didn’t fit into one of the above categories.

Multiline descriptions are allowed, but keep things as short as
possible (ideally one sentence); the CHANGELOG is not the place for
long explanations, that’s what the ticket system is for. If possible,
end each change line with a note in parentheses that says “Ticket #000
by John Doe” or “PR #000 by John Doe” depending on whether the change
was in response to a ticket on the tracker or was a pull request that
was merged. If a pull request itself refers to a ticket on the
tracker, combine it into a statement like “PR #000 by John Doe for
ticket #000 by William Shakespeare”. It is recommended to use the real
names in the ChangeLog rather than the nicknames of the persons
mentioned, because this makes it easier for those persons to point to
contributions they made. If the person in question demands not being
mentioned by real name, then of course honour that.

Take a look at the existing CHANGELOG file, the format is not as
complicated as it sounds. Also, Emacs has a ChangeLog major mode that
contains syntax highlighting for exactly this format.
