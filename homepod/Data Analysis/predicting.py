import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import tensorflow as tf
from sklearn.preprocessing import MinMaxScaler
from matplotlib.backends.backend_pdf import PdfPages


def load_data(file_path):
    data = pd.read_csv(file_path, parse_dates=[
                       'timestamp'], index_col='timestamp')
    # Datetime features
    data['hour'] = data.index.hour
    data['day_of_week'] = data.index.dayofweek
    data['day_of_year'] = data.index.dayofyear
    data['is_weekend'] = data.index.weekday >= 5
    for feature in ['temperature', 'humidity', 'moisture']:
        for lag in [1, 2, 3]:
            data[f'{feature}_lag{lag}'] = data[feature].shift(lag)
    data.dropna(inplace=True)
    return data


def make_rolling_predictions(model, feature_scaler, target_scaler, data, sequence_length, future_steps):
    future_predictions = []
    last_known_sequence = data[-sequence_length:]

    for _ in range(future_steps):
        current_sequence_scaled = feature_scaler.transform(last_known_sequence)
        prediction_scaled = model.predict(
            current_sequence_scaled[np.newaxis, :, :])
        prediction = target_scaler.inverse_transform(prediction_scaled)[0]
        future_predictions.append(prediction)

        # Update the last known sequence:
        # 1. Remove the oldest entry
        last_known_sequence = last_known_sequence[1:]
        # 2. Append the new prediction. This requires creating a new array that includes
        #    the prediction and other features like hour, day_of_week, etc., for the next timestamp.
        #    You might need to calculate or carry over these features for consistency.
        # Assuming last 3 features are what we're predicting
        next_sequence = np.append(last_known_sequence[-1][:-3], prediction)
        last_known_sequence = np.vstack([last_known_sequence, next_sequence])

    return np.array(future_predictions)


def plot_and_save(pdf, data, future_data, variables):
    for variable in variables:
        plt.figure(figsize=(14, 6))
        plt.plot(data.index, data[variable],
                 label=f'Historical {variable.capitalize()}', color='tab:blue', alpha=0.75)
        plt.plot(future_data.index, future_data[f'predicted_{variable}'],
                 label=f'Predicted {variable.capitalize()}', color='tab:orange', linestyle='--')
        plt.title(f'{variable.capitalize()} Over Time')
        plt.ylabel(f'{variable.capitalize()}')
        plt.xlabel('Time')
        plt.legend()
        plt.grid(True)
        pdf.savefig()
        plt.close()


data = load_data('historical_data.csv')
model = tf.keras.models.load_model('weather_prediction_model.h5')

# Load the scalers
feature_scaler = MinMaxScaler(feature_range=(0, 1))
feature_scaler.scale_ = np.load('feature_scaler_scale.npy')
feature_scaler.min_ = np.load('feature_scaler_min.npy')
target_scaler = MinMaxScaler(feature_range=(0, 1))
target_scaler.scale_ = np.load('target_scaler_scale.npy')
target_scaler.min_ = np.load('target_scaler_min.npy')

sequence_length = 24
feature_columns = ['hour', 'day_of_week', 'day_of_year', 'is_weekend'] + \
                  [f'{feat}_lag{lag}' for feat in ['temperature',
                                                   'humidity', 'moisture'] for lag in [1, 2, 3]]
last_known_data = data[-sequence_length:][feature_columns]
last_known_data = feature_scaler.transform(last_known_data)

# Predictions
future_steps = 365 * 24
future_predictions = make_rolling_predictions(
    model, feature_scaler, target_scaler, last_known_data, sequence_length, future_steps)

start_time = data.index[-1] + pd.Timedelta(hours=1)
future_timestamps = pd.date_range(
    start=start_time, periods=future_steps, freq='H')

future_data = pd.DataFrame(future_predictions, columns=[
                           'predicted_temperature', 'predicted_humidity', 'predicted_moisture'], index=future_timestamps)

with PdfPages('historical_and_predicted_soil_conditions.pdf') as pdf:
    plot_and_save(pdf, data, future_data, [
                  'temperature', 'humidity', 'moisture'])

print("Finished. The plots have been saved to 'historical_and_predicted_soil_conditions.pdf'.")
