import pandas as pd
import matplotlib.pyplot as plt

#
# Carrega CSV
#
# Formato esperado:
# pulso_us,pulso_ms,duty,duty_percent
#

arquivo = "notebook/dados.csv"

df = pd.read_csv(
    arquivo,
    header=None,
    names=[
        "pulso_us",
        "pulso_ms",
        "duty",
        "duty_percent"
    ],
    sep='\t'
)

#
# Cria eixo de tempo
#
# Cada amostra representa um log
#

df["tempo"] = range(len(df))

#
# ==============================
# GRÁFICO 1
# ==============================
#
# Pulso em ms pelo tempo
# PWM % no eixo secundário
#

fig, ax1 = plt.subplots(figsize=(12, 6))

#
# Eixo principal
#

ax1.plot(
    df["tempo"],
    df["pulso_ms"],
    linewidth=2,
    label="Pulso (ms)",
    color='red'
)

ax1.set_xlabel("Tempo (amostras)")
ax1.set_ylabel("Pulso (ms)")
ax1.legend()

#
# Eixo secundário
#

ax2 = ax1.twinx()

ax2.plot(
    df["tempo"],
    df["duty_percent"],
    linestyle="--",
    linewidth=2,
    label="PWM (%)"
)

ax2.set_ylabel("PWM (%)")
ax2.legend(loc="upper left")

#
# Título
#

plt.title("Pulso e PWM pelo Tempo")

#
# Grade
#

ax1.grid(True)

#
# ==============================
# GRÁFICO 2
# ==============================
#
# Pontos:
# Pulso (ms) x PWM (%)
#

plt.figure(figsize=(10, 6))

plt.scatter(
    df["duty_percent"],
    df["pulso_ms"]
)

plt.xlabel("PWM (%)")
plt.ylabel("Pulso (ms)")

plt.title("Pulso vs PWM")

plt.grid(True)

#
# ==============================
# GRÁFICO 3
# ==============================
#
# Média do pulso para cada duty
#

#
# Agrupa por duty
#

df_media = (
    df.groupby("duty_percent")["pulso_ms"]
    .mean()
    .reset_index()
)

#
# Ordena pelo duty
#

df_media = df_media.sort_values(
    by="duty_percent"
)

#
# Gera gráfico
#

plt.figure(figsize=(10, 6))

plt.plot(
    df_media["duty_percent"],
    df_media["pulso_ms"],
    marker='o',
    linewidth=2
)

plt.xlabel("PWM (%)")
plt.ylabel("Média do Pulso (ms)")

plt.title("Média do Pulso por PWM")

plt.grid(True)

#
# ==============================
# GRÁFICO 4
# ==============================
#
# Média do tempo do pulso (us)
# para cada duty
#

#
# Agrupa por duty
#

df_media_us = (
    df.groupby("duty_percent")["pulso_us"]
    .mean()
    .reset_index()
)

#
# Ordena pelo duty
#

df_media_us = df_media_us.sort_values(
    by="duty_percent"
)

#
# Gera gráfico
#

plt.figure(figsize=(10, 6))

plt.plot(
    df_media_us["duty_percent"],
    df_media_us["pulso_us"],
    marker='o',
    linewidth=2
)

plt.xlabel("PWM (%)")
plt.ylabel("Média do Tempo do Pulso (us)")

plt.title("Média do Tempo do Pulso por PWM")

plt.grid(True)

#
# ==============================
# GRÁFICO 1
# ==============================
#
# Média do pulso em ms pelo tempo
# PWM % no eixo secundário
#

#
# Calcula média do pulso por duty
#

df_media = (
    df.groupby("duty_percent")["pulso_ms"]
    .mean()
    .reset_index()
)

#
# Cria eixo de tempo real
#
# Cada passo ocorre a cada 5 segundos
#

df_media["tempo_s"] = (
    df_media.index * 5
)

#
# Cria figura
#

fig, ax1 = plt.subplots(figsize=(12, 6))

#
# Eixo principal
# Média do pulso
#

ax1.plot(
    df_media["tempo_s"],
    df_media["pulso_ms"],
    linewidth=2,
    marker='o',
    label="Média Pulso (ms)",
    color='red'
)

ax1.set_xlabel("Tempo (s)")
ax1.set_ylabel("Média do Pulso (ms)")

ax1.legend()

#
# Eixo secundário
# PWM %
#

ax2 = ax1.twinx()

ax2.plot(
    df_media["tempo_s"],
    df_media["duty_percent"],
    linestyle="--",
    linewidth=2,
    marker='s',
    label="PWM (%)"
)

ax2.set_ylabel("PWM (%)")
ax2.set_yticks(range(0, 101, 10))

ax2.grid(
    True,
    axis='y',
    linestyle='--',
    alpha=0.5
)
ax2.legend(loc="upper left")

#
# Título
#

plt.title("Média do Pulso e PWM pelo Tempo")

#
# Grade
#

ax1.grid(True)

#
# Exibe
#


plt.show()

