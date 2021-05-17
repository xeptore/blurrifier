import plotly.express as px
import pandas as pd

df = pd.read_csv("./results.csv")

df['time'] = df['time'] * 1e-9

pthread_fig = px.line(
    df,
    title="Kernel Radius Per Number of Workers Processing Times (pthread)",
    x="radius",
    y="time",
    color="workers",
    labels={ "time": "Duration (seconds)", "radius": "Kernel Radius" }
)

df = pd.read_csv("./openmp.csv")

df['time'] = df['time'] * 1e-9

openmp_fig = px.line(
    df,
    title="Kernel Radius Per Number of Workers Processing Times (OpenMP)",
    x="radius",
    y="time",
    color="workers",
    labels={ "time": "Duration (seconds)", "radius": "Kernel Radius" }
)

openmp_fig.show()
pthread_fig.show()