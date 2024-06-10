radio=$1
secto=$2
echo "Simulating r="$radio " sector="$secto

for counter in $(seq 1 1000); do
  seedv=$(( $RANDOM % 1000 + 1 ))
  runseedv=$(( $RANDOM % 1000 + 1 ))
  ./waf --run-no-build "randomdata8x8 --path=$PWD --radius=$radio --trial=$counter --sector=$secto --seed=$seedv --runseed=$runseedv" --cwd=./scratch/randomdata8x8/results/ >> ./results8x8/BC-random-positions-r=$radio-sector=$secto.txt 
done
echo "Finished r="$radio " sector="$secto

