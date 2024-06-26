import os
import numpy as np
import pandas as pd
from collections import defaultdict
import argparse

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--input_file", type=str, dest="input_file", required=True)
    args = parser.parse_args()

    base = os.path.basename(args.input_file)
    output_folder = os.path.dirname(args.input_file) + "/"
    filename = os.path.splitext(base)[0]

    data = pd.read_csv(output_folder + filename + ".txt", sep="\t", header=None)
    data.columns = [
        "x-ue",
        "y-ue",
        "z-ue",
        "vx-ue",
        "vy-ue",
        "vz-ue",
        "ue-codeword-ID",
        "gnb-codeword-ID",
        "Power",
        "Trial",
    ]

    # Beam pair for mimo configuration
    if "mimo=2x2" in filename:
        gnbcodewords = 3
        uecodewords = 2
    elif "mimo=4x4" in filename:
        gnbcodewords = 21
        uecodewords = 2
    else:
        gnbcodewords = 70
        uecodewords = 2

    pairmatrix = np.arange(0, gnbcodewords * uecodewords)
    pairmatrix = pairmatrix.reshape(uecodewords, gnbcodewords)
    data["Pair-beam"] = pairmatrix[
        data["ue-codeword-ID"].astype(int), data["gnb-codeword-ID"].astype(int)
    ]

    trials = data["Trial"].unique().tolist()

    dicts = []
    for trial in trials:
        subset = data[data["Trial"] == trial]
        # For each trial is search the highest power spectral density
        selected = subset.loc[subset["Power"].idxmax()]["Pair-beam"]
        dataf = defaultdict(list)
        # computing mean value
        dataf["Pair-beam"] = int(selected)
        dataf["x-ue"] = subset[subset["Pair-beam"] == int(selected)]["x-ue"].mean()
        dataf["y-ue"] = subset[subset["Pair-beam"] == int(selected)]["y-ue"].mean()
        dataf["z-ue"] = subset[subset["Pair-beam"] == int(selected)]["z-ue"].mean()
        dataf["vx-ue"] = subset[subset["Pair-beam"] == int(selected)]["vx-ue"].mean()
        dataf["vy-ue"] = subset[subset["Pair-beam"] == int(selected)]["vy-ue"].mean()
        dataf["vz-ue"] = subset[subset["Pair-beam"] == int(selected)]["vz-ue"].mean()
        dataf["Power"] = subset[subset["Pair-beam"] == int(selected)]["Power"].mean()
        dicts.append(dataf)

    finaldata = pd.DataFrame(dicts)
    finaldata = finaldata.reset_index(drop=True)
    filename = output_folder + filename
    output_path = filename + ".pqt"
    finaldata.to_parquet(output_path)


# Essa parte do codigo junta todos os arquivos processados para gerar um unico arquivo que vai
# ser usado para o treinamento. Para rodar essa parte, comente o codigo acima e rode o comando python3 datasector.py
# folder="data/8x8sector/"
# d1=pd.read_parquet(folder+"BC-random-positions-r=10.pqt")
# d2=pd.read_parquet(folder+"BC-random-positions-r=20.pqt")
# d3=pd.read_parquet(folder+"BC-random-positions-r=30.pqt")
# d4=pd.read_parquet(folder+"BC-random-positions-r=40.pqt")
# d5=pd.read_parquet(folder+"BC-random-positions-r=50.pqt")
# d6=pd.read_parquet(folder+"BC-random-positions-r=60.pqt")
# d7=pd.read_parquet(folder+"BC-random-positions-r=70.pqt")
# d8=pd.read_parquet(folder+"BC-random-positions-r=80.pqt")
# d9=pd.read_parquet(folder+"BC-random-positions-r=90.pqt")
# d10=pd.read_parquet(folder+"BC-random-positions-r=100.pqt")
# d11=pd.read_parquet(folder+"BC-random-positions-r=110.pqt")
# # esse arquivo são pontos aleatorios ao redor da BS. Coloquei para que o modelo saida dessa zona
# d12=pd.read_parquet(folder+"antenna.pqt")


# databases=[d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12]
# bigdata=pd.concat(databases)
# bigdata.reset_index(drop=True)
# bigdata.to_parquet(folder+"BC-random-positions.pqt")

# newdata=pd.read_parquet(folder+"BC-random-positions.pqt")
# print(newdata)
