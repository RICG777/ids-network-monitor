import io
import pigpio
import sys

def main():
    # Create a socket to listen for incoming packets.
    sock = pigpio.socket(pigpio.IF_RAW, pigpio.SOCK_RAW, pigpio.ETH_ALL)

    # Bind the socket to the Raspberry Pi's interface.
    sock.bind()

    # Create a file-like object to store the captured packets.
    pcap = io.StringIO()

    # Start capturing packets.
    while True:
        data = sock.recv(65536)
        pcap.write(data.decode('utf-8'))

    # Create a web page to display the captured packets.
    with open('packet_viewer.html', 'w') as f:
        f.write('<html><head><title>Packet Viewer</title></head><body>')
        for packet in pcap:
            f.write('<p>Packet:</p>')
            f.write('<pre>%s</pre>' % packet)
        f.write('</body></html>')

if __name__ == '__main__':
    main()