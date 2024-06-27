# bin/bash!

results_folder=results
datafolder=`ls ./$results_folder/*mimo**.txt`

for data in $datafolder
do
    echo "Opening" $(basename -- "$data")"..."
    # eliminate unseful text
    sed -i '/waf/d;/build/d;/\[/d;/Traceback/d;/run_commands/d;/ctx/d;/return/d;/m(self)/d;/root/d;/File/d;/obj/d;/parse_constant/d;/json/d;/raise/d;' $results_folder/$(basename -- "$data")
    python3 scratch/beampairdata/process_data_by_sector.py --input_file $results_folder/$(basename -- "$data")
    echo "File terminated..."
    
done

python3 scratch/beampairdata/process_data_by_ring.py --input_folder $results_folder
python3 scratch/beampairdata/process_data_all_rings.py --input_folder $results_folder