@echo off
"Prisoner's dilemma.exe" --payoffs 5 3 1 0 --rounds 50 --repeats 5  --seed 42 --epsilon 0.1 --strategies PROBER AllCooperate TitForTat PAVLOV GrimTrigger AllDefect
pause

