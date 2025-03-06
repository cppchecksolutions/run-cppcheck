# run-cppcheck

A wrapper for cppcheck to simplify cppcheck integration. This program exists to provide a way
to configure how to run cppcheck on a single file, that can be shared between editor plugins.
The log file also provides additional information that plugins may be lacking.

## Invocation

```console
run-cppcheck <file>
```

## Configuration options
The configuration file is written in json, as **run-cppcheck-config.json**. It is searched for recursively in parent directories of the analyzed file.

- **project_file**: a path to a project file accepted by the cppcheck option **--project=...**. If the path is relative,
                    it's interpreted as relative to the configuration file.
- **cppcheck**: cppcheck command.
- **log_file**: Path to log file. The default log file location is `$XDG_STATE_HOME/run-cppcheck` or `$HOME/.local/state/run-cppcheck` on Linux,
                and `%LOCALAPPDATA%\run-cppcheck` on Windows. The log file may provide more information than the editor plugin if analysis fails.
- **enable_logging**: Default is `true`.
- **extra_args**: Extra arguments to pass to cppcheck. Example: `["--enable=style"]`.
