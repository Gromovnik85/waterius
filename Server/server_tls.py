# -*- coding: utf-8 -*-
from __future__ import print_function  # для единого кода Python2, Python3

from werkzeug import serving
import ssl
from flask import Flask, request
from flask import jsonify

app = Flask(__name__)

host = '192.168.50.10'
port = 5000

"""
Настроим защищенную передачу данных от Вотериуса. 
Создадим Центр сертификации и сгенерируем ключ/сертификат для сервера.
В Вотериус запишем сертификат Центра сертификации. Он подтвердит, что сервер тот, за кого себя выдает.

openssl genrsa -out ca_key.pem 2048
openssl req -x509 -new -nodes -key ca_key.pem -days 4096 -out ca_cer.pem
openssl genrsa -out server_key.pem 2048

# Замените 192.168.1.10 на свой домен или IP адрес сервера (!). Именно его и подтверждает сертификат.

openssl req -out server_req.csr -key server_key.pem -new -subj '/CN=192.168.1.10/C=RU/ST=Moscow/L=Moscow/O=Waterius LLC/OU=Waterius community/emailAddress=your@email.ru'
# для windows: openssl req -out server_req.csr -key server_key.pem -new -subj '//CN=192.168.1.10\C=RU\ST=Moscow\L
=Moscow\O=Waterius LLC\OU=Waterius community\emailAddress=your@email.ru'

openssl x509 -req -in server_req.csr -out server_cer.pem -sha256 -CAcreateserial -days 4000 -CA ca_cer.pem -CAkey ca_key.pem


ca_key.pem - ключ вашего Центра сертификации. Храним в сейфе.
ca_cer.pem - публичный X.509 сертификат Центра сертификации для генерации ключей сервера, служит и для проверки публичного ключа сервера.
server_key.pem - приватный ключ сервера. Только на сервере.
server_cer.pem - публичный ключ сервера. Может быть передан клиентам.

В память Вотериуса записываем ca_cer.pem . Теперь Вотериус может по передавать данные зашифровано, убеждаясь, что сервер настоящий.
Сертификат имеет срок годности и требует обновления.
"""

# Сертификат https://letsencrypt.org/certs/lets-encrypt-x3-cross-signed.pem.txt
cacert2 = '''-----BEGIN CERTIFICATE-----
MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow
SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT
GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC
AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF
q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8
SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0
Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA
a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj
/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T
AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG
CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv
bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k
c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw
VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC
ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz
MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu
Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF
AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo
uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/
wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu
X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG
PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6
KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==
-----END CERTIFICATE-----'''


@app.route('/', methods=['POST'])
def root():
    """
    Print Waterius values
    """
    try:
        j = request.get_json()
        print('ch 0: %d' % j['ch0'])
        print('ch 1: %d' % j['ch1'])
        print('ca crc: %d' % j['ca_crc'])
        print('ca2 crc: %d' % j['ca2_crc'])
    except Exception as err:
        return jsonify({'status': 'ERROR'}), 400

    d = {'status': 'OK'}
    if j['ca2_crc'] == '0':
        d.update({'ca2': cacert2})
    return jsonify(d)


@app.route('/ping', methods=['GET'])
def ping():
    """
    Check server by sending GET request by Curl:

    curl https://192.168.1.42:5000/ping --cacert ./certs/ca_cer.pem
    """
    return 'pong'


if __name__ == "__main__":
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    #context.verify_mode = ssl.CERT_REQUIRED
    context.load_verify_locations("certs/ca_cer.pem")
    context.load_cert_chain("certs/server_cer.pem", "certs/server_key.pem")
    serving.run_simple(host, port, app, ssl_context=context)
