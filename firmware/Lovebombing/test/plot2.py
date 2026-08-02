import pandas as pd
import matplotlib.pyplot as plt

# ==========================================================
# Leitura do arquivo
# ==========================================================
arquivo = "test/transiente_02_020826.csv"
# arquivo = "test/pulsatil_02_020826.csv"

df = pd.read_csv(arquivo, sep=';')

plt.rcParams.update({
    "text.usetex": True,
    "font.family": "serif",
    "mathtext.fontset": "cm",
    "font.serif": ["cmr10", "DejaVu Serif", "serif"],
    "axes.formatter.use_mathtext": True,

    # Fontes
    "font.size": 20,
    "axes.titlesize": 22,
    "axes.labelsize": 20,
    "xtick.labelsize": 18,
    "ytick.labelsize": 18,
    "legend.fontsize": 18,
    "figure.titlesize": 24,

    # Espessura dos eixos
    "axes.linewidth": 1.2,

    # Tamanho dos ticks
    "xtick.major.size": 6,
    "ytick.major.size": 6,
    "xtick.major.width": 1.2,
    "ytick.major.width": 1.2,
})

# ==========================================================
# Dados
# ==========================================================
tempo = df["tempo(us)"] / 1000.0      # Tempo em ms
vazao = df["pressao(kPa)"]
duty = df["duty(%)"]

# ==========================================================
# Gráfico
# ==========================================================
fig, ax1 = plt.subplots(figsize=(15, 7))

# Eixo Y primário (azul)
ax1.plot(tempo, vazao, label="kPa", linewidth=2, color="blue")
ax1.set_xlabel(r"Tempo (ms)")
ax1.set_ylabel(r"Pressão (kPa)", color="blue")
ax1.tick_params(axis='y', colors='blue')
ax1.spines['left'].set_color('blue')
ax1.yaxis.label.set_color('blue')
ax1.grid(True)

# Eixo Y secundário (preto)
ax2 = ax1.twinx()
ax2.plot(tempo, duty, '--', linewidth=2, label=r"Duty (\%)", color='black')
ax2.set_ylabel(r"Duty (\%)", color='black')
ax2.tick_params(axis='y', colors='black')
ax2.spines['right'].set_color('black')
ax2.yaxis.label.set_color('black')
ax2.set_ylim(0, 100)

# ==========================================================
# Legenda única
# ==========================================================
linhas1, labels1 = ax1.get_legend_handles_labels()
linhas2, labels2 = ax2.get_legend_handles_labels()

# ax1.legend(linhas1 + linhas2, labels1 + labels2, loc="lower right")
ax1.legend(linhas1 + linhas2, labels1 + labels2, loc="upper right")

plt.tight_layout()

# plt.savefig("test/tempoXpressaoXdutypercent_pulsatil.pdf")
plt.savefig("test/tempoXpressaoXdutypercent_transiente.pdf")

plt.show()