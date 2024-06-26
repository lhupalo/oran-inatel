# bin/bash!

# teste individual por arquivo 
# sed -i '/waf/d;/build/d;/\[/d;/Traceback/d;/run_commands/d;/ctx/d;/return/d;/m(self)/d;/root/d;/File/d;/obj/d;/parse_constant/d;/json/d;/raise/d;' ../xad_steering_beamforming/data/8x8blockage/BC-random-positions-sector-2-r=30.txt

# python3 datasector.py ../xad_steering_beamforming/data/8x8blockage2/BC-random-positions-r=70.txt ../xad_steering_beamforming/data/8x8blockage2/ 

# fazendo o processamento em todos os arquivos na pasta, no casso 8x8sector

# datafolder=`ls ~/Documentos/Inatel/XAD/repositories/xad_steering_beamforming/data/8x8sector/*.txt`
fder=$1

datafolder=`ls ./$fder/*.txt`

echo $datafolder

for data in $datafolder
do
    echo "Opening" $(basename -- "$data")"..."
    # eliminate unseful text
    sed -i '/waf/d;/build/d;/\[/d;/Traceback/d;/run_commands/d;/ctx/d;/return/d;/m(self)/d;/root/d;/File/d;/obj/d;/parse_constant/d;/json/d;/raise/d;' ./$fder/$(basename -- "$data")
    python3 datasector.py ./$fder/$(basename -- "$data") ./$fder/ 
    echo "File terminated..."
    
done