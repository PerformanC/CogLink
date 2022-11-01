---
name: Bug report
about: Help Coglink be more stable by reporting bugs
title: '[bug]: '
labels: bug
assignees: ''
---

## What's wrong?

Please inform us as detailed as possible here what happened.

## Reproducing

Let us know how to reproduce this bug.

## Crashes (debug)

If Coglink crashed, we need to first discover where it came from, so we can fix it. To do that, you will need to compile Coglink with debug mode, and then run it with the `-g` flag, and then send us the output. After that, you can execute this command:

```console
$ gdb ./ConcordBot
```

Then, press `c` and `enter`, and now that you are on GDB's shell, please use the command `run`.

After that, your bot using Coglink will be running, then please reproduce the bugs and send us the output.

```text
# Output here
```

## Screenshots

If possible, let a screenshot here, if not, please remove this section.

## Checkmarks

- [ ] I confirm that I've tried reproducing this bug on the latest release and I'm still able to reproduce.