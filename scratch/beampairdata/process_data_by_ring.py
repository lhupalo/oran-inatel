import pandas as pd
import argparse
import glob

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--input_folder", type=str, dest="input_folder", required=True)
    args = parser.parse_args()

    mimos = ["2x2", "4x4", "8x8"]

    for mimo in mimos:
        files = glob.glob(
            args.input_folder + "/*mimo=" + mimo + "**.pqt", recursive=True
        )
        if len(files) != 0:
            radius = [word[word.index("r=") :][2:5] for word in files]
            radius = [word.replace("_", "") for word in radius]
            radius = list(set(radius))
            for radio in radius:
                database = []
                for file in files:
                    if radio in file:
                        database.append(pd.read_parquet(file))
                print(len(database))
                bigdata = pd.concat(database)
                bigdata.reset_index(drop=True)
                bigdata.to_parquet(
                    args.input_folder
                    + "/beam_pair_data_mimo="
                    + mimo
                    + "_r="
                    + radio
                    + ".pqt"
                )
        else:
            print("Not found beam pair data for sectors in mimo " + mimo)
