import plotly.express as px
import plotly.io as pio
import pandas as pd

pio.kaleido.scope.default_width = 700 * 1
pio.kaleido.scope.default_height = 500 * 1
pio.kaleido.scope.default_width = 900 * 1
pio.kaleido.scope.default_height = 1200 * 1
pio.kaleido.scope.default_scale = 5

pthread_dataframe = pd.read_csv("./results.csv")

pthread_dataframe['time'] = pthread_dataframe['time'] * 1e-9

pthread_fig = px.line(
    pthread_dataframe,
    title="Processing Time For Different Kernel Radii (pthread)",
    x="radius",
    y="time",
    color="Workers",
    labels={"time": "Duration (seconds)", "radius": "Kernel Radius"},
    range_x=[2, 20],
    range_y=[0, 700]
)
pthread_fig.write_image(f"./benchmark_diagrams/pthread.png")

pthread_dataframe["Threading"] = "pthread"

openmp_dataframe = pd.read_csv("./openmp.csv")
openmp_dataframe['time'] = openmp_dataframe['time'] * 1e-9

openmp_fig = px.line(
    openmp_dataframe,
    title=f"Processing Time For Different Kernel Radii (OpenMP)",
    x="radius",
    y="time",
    color="Workers",
    labels={"time": "Duration (seconds)", "radius": "Kernel Radius"},
    range_x=[2, 20],
    range_y=[0, 700]
)
openmp_fig.write_image(f"./benchmark_diagrams/openmp.png")

openmp_dataframe["Threading"] = "OpenMP"

for i in range(32):
    fig = px.line(
        pd.concat([pthread_dataframe.where(pthread_dataframe["Workers"] == i + 1).loc[i*10:(i+1) * 10 - 1],
                   openmp_dataframe.where(openmp_dataframe["Workers"] == i + 1).loc[i*10:(i+1) * 10 - 1]]),
        title=f"Processing Time For Different Kernel Radii Using {i + 1} Worker{'s' if i > 0 else ''}",
        x="radius",
        y="time",
        color="Threading",
        labels={"time": "Duration (seconds)", "radius": "Kernel Radius"},
        range_x=[2, 20],
        range_y=[0, 700]
    )
    fig.write_image(f"./benchmark_diagrams/{i + 1:02}w.png")
