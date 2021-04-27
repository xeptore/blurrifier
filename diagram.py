import plotly.express as px
import pandas as pd

df = pd.read_csv("./results.csv")

df['time'] = df['time'] * 1e-9

fig = px.line(
    df,
    title="Kernel Radius Per Number of Workers Processing Times",
    x="radius",
    y="time",
    color="workers",
    labels={ "time": "Duration (seconds)", "radius": "Kernel Radius" }
)
fig.show()
