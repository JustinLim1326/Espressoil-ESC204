import pandas as pd
import numpy as np
from sklearn.preprocessing import MinMaxScaler
import tensorflow as tf
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

data = pd.read_csv('historical_data.csv', parse_dates=[
                   'timestamp'], index_col='timestamp')

# Feature Engineering
data['hour'] = data.index.hour
data['day_of_week'] = data.index.dayofweek
data['day_of_year'] = data.index.dayofyear
data['is_weekend'] = data.index.weekday >= 5

# Lag Features
for lag in [1, 2, 3]:
    data[f'temperature_lag{lag}'] = data['temperature'].shift(lag)
    data[f'humidity_lag{lag}'] = data['humidity'].shift(lag)
    data[f'moisture_lag{lag}'] = data['moisture'].shift(lag)

data = data.dropna()

target_columns = ['temperature', 'humidity', 'moisture']
feature_columns = [col for col in data.columns if col not in target_columns]
features = data[feature_columns].values
targets = data[target_columns].values

# scaling! what the fuck does this do? idk
feature_scaler = MinMaxScaler(feature_range=(0, 1))
target_scaler = MinMaxScaler(feature_range=(0, 1))
scaled_features = feature_scaler.fit_transform(features)
scaled_targets = target_scaler.fit_transform(targets)


def create_sequences(features, targets, sequence_length=24):
    X, y = [], []
    for i in range(len(features) - sequence_length):
        X.append(features[i:i + sequence_length])
        y.append(targets[i + sequence_length])
    return np.array(X), np.array(y)


sequence_length = 24
X, y = create_sequences(scaled_features, scaled_targets, sequence_length)

# Splitting the data for training and testing
split_time = -30*24
X_train, X_test = X[:split_time], X[split_time:]
y_train, y_test = y[:split_time], y[split_time:]

# Model building
model = tf.keras.Sequential([
    tf.keras.layers.LSTM(128, input_shape=(
        sequence_length, X.shape[2]), return_sequences=True),
    tf.keras.layers.Dropout(0.2),
    tf.keras.layers.LSTM(64, return_sequences=False),
    tf.keras.layers.Dropout(0.2),
    tf.keras.layers.Dense(y.shape[1])
])
model.compile(optimizer=tf.keras.optimizers.Adam(
    learning_rate=0.001), loss='mse')

early_stopping = tf.keras.callbacks.EarlyStopping(
    monitor='val_loss', patience=5, restore_best_weights=True)

history = model.fit(X_train, y_train, epochs=50, batch_size=32, validation_data=(
    X_test, y_test), verbose=1, callbacks=[early_stopping])

# Save the model and scalers
model.save('weather_prediction_model.h5')

# what the fuck are these

np.save('feature_scaler_scale.npy', feature_scaler.scale_)
np.save('feature_scaler_min.npy', feature_scaler.min_)


np.save('target_scaler_scale.npy', target_scaler.scale_)
np.save('target_scaler_min.npy', target_scaler.min_)

# Plot training and validation loss
with PdfPages('training_validation_loss.pdf') as pdf:
    plt.figure(figsize=(10, 6))
    plt.plot(history.history['loss'], label='Training Loss')
    plt.plot(history.history['val_loss'], label='Validation Loss')
    plt.title('Model Loss During Training')
    plt.xlabel('Epoch')
    plt.ylabel('Loss')
    plt.legend()
    pdf.savefig()
    plt.close()

print("Training completed. Model and scalers have been saved.")
