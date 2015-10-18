# git-store: Using git as a KV store for file system

## Motivation
When I do some computational experiments with different parameters, I want to
records my parameters settings and result log file. Git itself can do that, but
in this scenario, concepts like commit, tag, branch are not very useful. And
most importantly, I don’t want to see all these files clutter my working trees.
I just need a way to reference these files.

Git stores content of files, directories (trees) as objects in
“.git””,directory, and these objects are not necessarily associated with
commits. By using some lower-level plumbing commands we can make git a KV store
for file system.

*When using this utility, it’s important to keep the repoistroy clean,. You init
a git repo and never commit things into it. Because when we write tree into git,
  we have to utilize git staging area (index). Currently we assume this staging
  area has nothing.*

## Commands
  - init
      - run `git init` and `git config gc.pruneExpire never` to prevents some
        git commands automatically prune unrechable objects.
  - write
      - write a content into “.git”, could be a file or a directory, or multiple
        files & directories.
      - options:
          - --tree,-t: Write as a tree, even only a single file is given.
          - --delete,-d: Delete the given files & directories after writing.
  - read
      - With a SHA-1 hash of a tree, it will read the tree into current
        directory.
      - With a SHA-1 hash of a blob object (single file), it will print it’s
        content into stdout.

## Credits
- Files inside `boost` and `libs` come from [boost
  library](http://www.boost.org/). I extract these files out so that users
  doesn’t need to compile boost library themselves.
