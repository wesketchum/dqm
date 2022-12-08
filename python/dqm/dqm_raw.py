from dqm.import_checks import kafka, msgpack, numpy as np

def main(arg, channels, planes, run_number, partition, app_name, kafka_address, topic):
    adc = arg.get_adc()
    all_raw = []
    all_channels = []
    all_planes = []
    print(len(adc), len(channels), len(planes))
    for i in range(len(adc)):
        if len(adc[i]) and len(channels[i]) and len(planes[i]):
            all_raw.append(adc[i])
            all_channels.append(channels[i])
            all_planes.append(planes[i])
    all_raw = np.concatenate(all_raw)
    all_channels = np.concatenate(all_channels).reshape((-1, 1))
    all_planes = np.concatenate(all_planes).reshape((-1, 1))
    all_values = np.concatenate((all_planes, all_channels, all_raw.T), axis=1)
    # Sort by plane, channel
    all_values = all_values[np.lexsort(all_values.T[::-1])]
    # print(all_values.shape)
    index_1 = np.searchsorted(all_values[:, 0], 1)
    index_2 = np.searchsorted(all_values[:, 0], 2)
    indexes = [0, index_1, index_2, all_values.shape[0]]
    producer = kafka.KafkaProducer(bootstrap_servers=kafka_address) if kafka_address else None
    for i in range(3):
        print(i)
        channels = all_values[indexes[i]:indexes[i+1], 1].tolist()
        values = all_values[indexes[i]:indexes[i+1], 2:].flatten().tolist()

        source, run_number, partition, app_name, plane, algorithm = '', run_number, partition, app_name, i, 'raw'

        msg = f'''{{"source": "{source}", "run_number": "{run_number}", "partition": "{partition}", "app_name": "{app_name}", "plane": "{plane}", "algorithm": "{algorithm}" }}'''.encode()
        msg += '\n\n\nM'.encode()
        channels = msgpack.packb(channels)
        msg += channels + '\n\n\nM'.encode()
        values = msgpack.packb(values)
        msg += values
        if producer:
            print(f'Sending message with length {len(msg)}')
            producer.send(topic, msg)
        else:
            print(f'Would send message with length {len(msg)}')
