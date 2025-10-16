@echo off
"Prisoner's dilemma.exe" --payoffs 5 3 1 0 --rounds 50 --repeats 5  --seed 42 --noise_sweep --epsilon_values 0.0 0.05 0.1 0.15 0.20 --strategies TitForTat GrimTrigger PAVLOV ContriteTitForTat
pause

