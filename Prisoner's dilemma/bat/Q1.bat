@echo off
"Prisoner's dilemma.exe" --payoffs 5 3 1 0 --rounds 10 --repeats 2 --seed 42 --strategies AllCooperate AllDefect TitForTat GrimTrigger PAVLOV --format csv
pause
