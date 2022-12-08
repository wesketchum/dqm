import numpy as np

def main(arg, channels, planes, run_number, partition, app_name):
    adc = arg.get_adc()
    all_rms = []
    all_channels = []
    all_planes = []
    print(len(adc), len(channels), len(planes))
    for i in range(len(adc)):
        print('ADC', adc[i])
        print('Channels: ', channels[i])
        print('Planes: ', planes[i])
        if len(adc[i]) and len(channels[i]) and len(planes[i]):
            all_rms.append( np.sqrt((adc[i] ** 2).mean(axis=0)) )
            all_channels.append(channels[i])
            all_planes.append(planes[i])
    all_rms = np.concatenate(all_rms).reshape((-1, 1))
    all_channels = np.concatenate(all_channels).reshape((-1, 1))
    all_planes = np.concatenate(all_planes).reshape((-1, 1))
    all_values = np.concatenate((all_planes, all_channels, all_rms), axis=1)
    # Sort by plane, channel
    all_values = all_values[np.lexsort(all_values.T[::-1])]
    # print(all_values.shape)
    index_1 = np.searchsorted(all_values[:, 0], 1)
    index_2 = np.searchsorted(all_values[:, 0], 2)
    indexes = [0, index_1, index_2, all_values.shape[0]]
    for i in range(3):
        print(i)
        channels = all_values[indexes[i]:indexes[i+1], 1]
        values = all_values[indexes[i]:indexes[i+1], 2]
        print(channels.shape, values.shape)
        print(list(channels), list(values))

        try:
            from kafka import KafkaProducer
            import msgpack
        except ModuleNotFoundError:
            print('kafka is not installed')
        producer = KafkaProducer(bootstrap_servers='monkafka:30092')
        source, run_number, partition, app_name, plane, algorithm = '', run_number, partition, app_name, i, 'std'

        msg = f'''{{"source": "{source}", "run_number": "{run_number}", "partition": "{partition}", "app_name": "{app_name}", "plane": "{plane}", "algorithm": "{algorithm}" }}'''.encode()
        msg += '\n\n\nM'.encode()
        channels = msgpack.packb(list(channels))
        msg += channels + '\n\n\nM'.encode()
        values = msgpack.packb(list(values))
        msg += values
        print(f'Sending message with length {len(msg)}')
        producer.send('DQM', msg)
