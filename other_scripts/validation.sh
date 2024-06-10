./waf --run "cb-bf-com" --cwd=./scratch/cb-bf-com/results/ > ../xad_steering_beamforming/data/validation/onlyns32.txt
# NS_LOG="MmWaveBeamformingModel=level_all|prefix_func|prefix_time:OpenGymInterface=level_all|prefix_func|prefix_time" ./waf --run "cb-bf-com" --cwd=./scratch/cb-bf-com/results/
# NS_LOG="*=all|prefix_level" ./waf --run "cb-bf-com" --cwd=./scratch/cb-bf-com/results/
# *=all|prefix_level