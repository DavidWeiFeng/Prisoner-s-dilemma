@echo off
"Prisoner's dilemma.exe" --evolve --generations 25 --strategies AllCooperate AllDefect TitForTat PAVLOV ContriteTitForTat --rounds 100 --repeats 5 --epsilon 0.05 --enable-scb --scb-cost 0.2
pause
