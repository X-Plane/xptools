X-Plane Scenery Tools README
====================================================================

The X-Plane Scenery Tools (XPTools) code base is the source code tree for all 
of the Laminar Research scenery creation/editing tools. This does not 
include X-Plane, Plane Maker, or Airfoil Maker.  It does include source to WorldEditor (WED),
and our global scenery generator RenderFarm, and other tools.

Contents
-------------------------------------------------------------------------------

- [Licensing and Copyright](#licensing-and-copyright)
- [Building the Applications](#building-the-applications)
- [Contributing Using Git & GitHub](#contributing-using-git--github)
- [Top Level File Structure](#top-level-file-structure)
- [Documentation](#documentation)
- [Mailing List/Contact](#mailing-listcontact)

Licensing and Copyright
-------------------------------------------------------------------------------

The code original to Laminar Research lives in the sub-directory "src" and is licensed
under the MIT/X11 license.  If you find a source file with no copyright, or double/conflicting
copyright, please report this (see contact info below)—this is probably a clerical error.

[Building the Applications](Building.md)
-------------------------------------------------------------------------------

See [Building.md](Building.md) for setup, build instructions, and dev environment setup.

We do our best to keep `master` building all projects and in general be release-ready,
but to get a stable release, use a tag associated with some kind of beta or release milestone.


Contributing Using Git & GitHub
-------------------------------------------------------------------------------

If you’d like to contribute to the project, you can do so by forking the repo on GitHub and making a pull request. (If you’re new to Git or GitHub, have a look at [the GitHub guides](https://guides.github.com), especially “Hello World” and the Git Handbook.)

In general, the repo’s `master` branch reflects the current state of development, while release branches are used for staging and patching binary releases (so, for instance, `wed_230_release` is the release branch for WED version 2.3). There are also corresponding tags for public releases (e.g., `wed_231r1`).

Starting a new development branch based on the tip of `master` is probably a good idea to avoid merge conflicts. I encourage the use of `git rebase` after pulling new changes and updating the master branch to have your local development branch up-to-date, unless you have people pulling from your repository. In that case, merging `master` back to your development branch is a better choice because rebasing causes the creation of new commits with new SHA1 checksums which might distort the workflow of users pulling from your repository.

Once you’ve finished your work and you think it’s time to submit your changes, you can use the GitHub UI to submit a pull request.

Top Level File Structure
-------------------------------------------------------------------------------

- cmake
    - All cmake scripts to build the various tools 
- src
    - The main source tree for the various tools and executable of XPTools.
- test
    - Collection of files for regression testing of WED
- scripts
    - A collection of scripts we use to package distros, and other things.
- SDK
    - The SDK for X-Plane