import pandas as pd
import matplotlib.pyplot as plt

# ==========================================================
# Leitura do arquivo CSV
# ==========================================================
arquivo = "test/transiente_01_020826.csv"

df = pd.read_csv(arquivo, sep=';')

# ==========================================================
# Tempo (eixo X)
# Converte de us para ms (opcional)
# ==========================================================
tempo = df["tempo(us)"] / 1000.0

# ==========================================================
# Gera um gráfico para cada coluna
# ==========================================================
for coluna in df.columns[1:]:

    plt.figure(figsize=(10, 4))

    plt.plot(tempo, df[coluna], linewidth=1.2)

    plt.title(coluna)
    plt.xlabel("Tempo (ms)")
    plt.ylabel(coluna)

    plt.grid(True)

    plt.tight_layout()
    plt.show()