import numpy as np
from scipy.fft import rfft, rfftfreq
try:
    from kafka import KafkaProducer
    import msgpack
except ModuleNotFoundError:
    print('kafka is not installed')

def main(arg, channels, planes, run_number, partition, app_name):
    adc = arg.get_adc()
    all_raw = []
    all_planes = []
    for i in range(len(adc)):
        if len(adc[i]) and len(planes[i]):
            all_raw.append(adc[i])
            all_planes.append(planes[i])
    all_raw = np.concatenate(all_raw)
    all_planes = np.concatenate(all_planes).reshape((-1, 1))
    all_values = np.concatenate((all_planes, all_raw.T), axis=1)
    # Sort by plane
    all_values = all_values[np.lexsort(all_values.T[::-1])]
    index_1 = np.searchsorted(all_values[:, 0], 1)
    index_2 = np.searchsorted(all_values[:, 0], 2)
    indexes = [0, index_1, index_2, all_values.shape[0]]
    for i in range(3):
        print(i)
        values = all_values[indexes[i]:indexes[i+1], 1:].sum(axis=0)

        fft = np.abs(rfft(values))
        if i == 0:
            total_fft = fft
        else:
            total_fft += fft
        freqs = rfftfreq(len(values), 200e-9)

        producer = KafkaProducer(bootstrap_servers='monkafka:30092')
        source, run_number, partition, app_name, plane, algorithm = '', run_number, partition, app_name, i, 'fourier_plane'

        msg = f'''{{"source": "{source}", "run_number": "{run_number}", "partition": "{partition}", "app_name": "{app_name}", "plane": "{plane}", "algorithm": "{algorithm}" }}'''.encode()
        msg += '\n\n\nM'.encode()
        freqs = msgpack.packb(list(freqs))
        msg += freqs + '\n\n\nM'.encode()
        fft = msgpack.packb(list(fft))
        msg += fft
        print(f'Sending message with length {len(msg)}')
        producer.send('DQM', msg)

    producer = KafkaProducer(bootstrap_servers='monkafka:30092')
    source, run_number, partition, app_name, plane, algorithm = '', run_number, partition, app_name, 3, 'fourier_plane'

    msg = f'''{{"source": "{source}", "run_number": "{run_number}", "partition": "{partition}", "app_name": "{app_name}", "plane": "{plane}", "algorithm": "{algorithm}" }}'''.encode()
    msg += '\n\n\nM'.encode()
    # freqs = msgpack.packb(list(freqs))
    msg += freqs + '\n\n\nM'.encode()
    fft = msgpack.packb(list(total_fft))
    msg += fft
    print(f'Sending message with length {len(msg)}')
    producer.send('DQM', msg)
