@echo off
cloc ./ --exclude-ext="json,bat" --not-match-d=include
pause