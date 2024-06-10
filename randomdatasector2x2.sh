radio=$1
secto=$2
echo "Simulating r="$radio " sector="$secto

for counter in $(seq 1 1000); do
  seedv=$(( $RANDOM % 1000 + 1 ))
  runseedv=$(( $RANDOM % 1000 + 1 ))
  ./waf --run-no-build "randomdata2x2 --path=$PWD --radius=$radio --trial=$counter --sector=$secto --seed=$seedv --runseed=$runseedv" --cwd=./scratch/randomdata2x2/results/ >> ./results2x2/BC-random-positions-r=$radio-sector=$secto.txt
done
echo "Finished r="$radio " sector="$secto

