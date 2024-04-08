import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import griddata
from matplotlib.backends.backend_pdf import PdfPages

data = pd.read_csv('sensor_data.csv')


def create_gradient_map(data, variable, title, pdf):
    x, y, values = data['longitude'], data['latitude'], data[variable]
    X, Y = np.meshgrid(
        np.linspace(np.min(x), np.max(x), 1000),
        np.linspace(np.min(y), np.max(y), 1000)
    )
    interpolated_values = griddata((x, y), values, (X, Y), method='cubic')

    plt.figure(figsize=(10, 8))
    contour = plt.contourf(X, Y, interpolated_values, cmap='viridis')
    plt.scatter(x, y, c=values, edgecolors='w', s=50, cmap='viridis')
    cbar = plt.colorbar(contour)
    cbar.set_label(variable)
    plt.xlim(np.min(x), np.max(x))
    plt.ylim(np.min(y), np.max(y))
    plt.xlabel('Longitude')
    plt.ylabel('Latitude')
    plt.title(title)

    pdf.savefig()
    plt.close()


# Exporting graphs into pdf
with PdfPages('gradient_maps.pdf') as pdf:
    create_gradient_map(data, 'temperature', 'Temperature Gradient Map', pdf)
    create_gradient_map(data, 'humidity', 'Humidity Gradient Map', pdf)
    create_gradient_map(data, 'moisture', 'Moisture Level Gradient Map', pdf)
