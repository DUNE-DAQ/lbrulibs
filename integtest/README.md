"integtests" are intended to be automated integration and/or system tests that make use of the
"pytest" framework to validate the operation of the DAQ system in various scenarios.

Here is a sample command for invoking a test (feel free to keep or drop the options in brackets, as you prefer):

```
pytest -s test_mpd-raw.py [--nanorc-option partition-number 2] [--nanorc-option timeout 300]
```
