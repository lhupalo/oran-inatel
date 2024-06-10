radio=$1
secto=$2
echo "Simulating r="$radio " sector="$secto

# define inside for loop how much positions per sector and radius do you want. Here default 1000
for counter in $(seq 1 1000); do
  seedv=$(( $RANDOM % 1000 + 1 ))
  runseedv=$(( $RANDOM % 1000 + 1 ))
  ./waf --run-no-build "randomdata4x4 --path=$PWD --radius=$radio --trial=$counter --sector=$secto --seed=$seedv --runseed=$runseedv" --cwd=./scratch/randomdata4x4/results/ >> ./results4x4/BC-random-positions-r=$radio-sector=$secto.txt
done
echo "Finished r="$radio " sector="$secto

