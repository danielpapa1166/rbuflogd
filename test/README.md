# Producer Smoketest

Run:

```sh
./build/rbuflogd_producer_test [options]
```

Options:

- `-p`, `--producer NAME` set producer name (default: `prod1`)
- `-c`, `--category CATEGORY` set category (default: `cat1`)
- `-m`, `--message TEXT` set message text (default: `test message 0`)
- `-l`, `--log-level LEVEL` set log level (`debug`, `info`, `warning`, `error`; default: `info`)
- `-h`, `--help` show help

Example:

```sh
./build/rbuflogd_producer_test -p prod12345 -c network -m "hello" -l warning
```
