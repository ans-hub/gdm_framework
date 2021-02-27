@echo off

setlocal enableDelayedExpansion

:: asigning arguments as a key-value pairs
:: https://stackoverflow.com/questions/26551/how-can-i-pass-arguments-to-a-batch-file

echo Parsing args

set counter=0
for %%# in (%*) do (    
    set /a counter=counter+1
    set /a even=counter%%2
    
    if !even! == 0 (
        set "!prev!=%%~#"
    )
    set "prev=%%~#"
)

echo Create build tree

if not exist %_build_tree_% (
	mkdir %_build_tree_%
	cmake -B %_build_tree_% -S %_source_tree_%
)

echo Building

cmake --build %_build_tree_% --config %_config_%

endlocal