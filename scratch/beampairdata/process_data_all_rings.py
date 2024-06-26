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
            args.input_folder + "/*mimo=" + mimo + "*.pqt", recursive=True
        )
        files= [ x for x in files if "sector" not in x ]
        print(files)
        if len(files) != 0:
            database = []
            for file in files:
                database.append(pd.read_parquet(file))
            print(len(database))
            bigdata = pd.concat(database)
            bigdata.reset_index(drop=True)
            bigdata.to_parquet(
                args.input_folder
                + "/beam_pair_data_mimo="
                + mimo
                + ".pqt"
            )
        else:
            print("Not found beam pair data for sectors in mimo " + mimo)