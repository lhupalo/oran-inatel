# radius=(30 40 50 60 70 80 90 100)
# radius=(30 40 50 60)
radius=(30)

for radio in ${radius[@]}; 
do
  echo "Simulating r="$radio"..."
  # ./waf --run "codebook-bf --radius=$radio" --cwd=./scratch/codebook-bf/results/ > ../xad_steering_beamforming/data/2x2codebook/BC-random-positions-r=$radio.txt 
  ./waf --run "codebook-bf --radius=$radio" --cwd=./scratch/codebook-bf/results/
  # ./waf --run "cb-bf --radius=$radio" --cwd=./scratch/cb-bf/results/ > ../xad_steering_beamforming/data/2x2codebook/BC-random-positions-r=$radio.txt
  echo "Finished r="$radio"..."
done

