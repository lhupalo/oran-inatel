# radius=(30 40 50 60 70 80 90 100)
radius=(30)

for radio in ${radius[@]}; 
do
  # ./waf --run "codebook-bf-2 --radius=$radio" --cwd=./scratch/codebook-bf-2/results/ >> velocities/random-velocities-r=$radio.txt
  # ./waf --run "randomdata --radius=$radio" --cwd=./scratch/randomdata/results/ > ../xad_steering_beamforming/data/2x2codebook/BC-random-positions-r=$radio.txt 
  ./waf --run "randomdata --radius=$radio" --cwd=./scratch/randomdata/results/
  # echo "Finished r="$radio
done

