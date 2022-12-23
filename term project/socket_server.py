
import socket 
import json
import requests as req
import time

HOST = "HOST IP Address" 
PORT = 6531 
sos_flag = 0
sos_status = 0
step_count = 0
step_flag = 0
step_threshold = 20000
fall_status = 0
fall_flag = 0
fall_threshold = 1700
heart_flag = 0
n = 0
heartrate_tosend = 80


def fall_detection(acc, accx, accy, accz):
    global fall_flag, fall_status
    if acc > fall_threshold and fall_flag == 0 and abs(accx) < 2000 and abs(accy) < 2000 and abs(accz) < 2000:
        fall_status = 1
        fall_flag = 1
    elif acc <= fall_threshold and fall_flag == 1:
        fall_flag = 0
        fall_status = 0

def sos_call(sos):
    global sos_flag, sos_status
    if sos == 1 and sos_flag == 1:
        sos_status = 1
        sos_flag = 0
    elif sos == 1 and sos_flag == 0:
        sos_flag = 1


def pedometer(gx, gy, gz, accx):
    global step_flag, step_count
    if gz > step_threshold and step_flag == 0 and abs(gx) < 80000 and abs(gy) < 80000 and accx > 500:
        step_count += 1
        print(step_count)
        step_flag = 1
    elif gz <= step_threshold and step_flag == 1:
        step_flag = 0


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print("Starting server at: ", (HOST, PORT))
    conn, addr = s.accept()
    with conn:
        step_count = 0
        print("Connected at", addr)
        while True:
            data = conn.recv(1024)
            try:
                obj = json.loads(data)
            except:
                continue
            sos = obj['sos']
            acc = obj['acc']
            acc_x = obj['acc_x']
            acc_y = obj['acc_y']
            acc_z = obj['acc_z']
            gyro_x = obj['gyro_x']
            gyro_y = obj['gyro_y']
            gyro_z = obj['gyro_z']
            heartrate = obj['heartrate']
            

            fall_detection(acc, acc_x, acc_y, acc_z)
            pedometer(gyro_x, gyro_y, gyro_z, acc_x)
            sos_call(sos)

            if heartrate<60 or heartrate>100:
                heart_flag += 1
            
            if heart_flag == 20 or 100 > heartrate > 60:
                heart_flag = 0
                heartrate_tosend = heartrate


            if n > 20 and n % 5 == 0:
                url = f'https://f9e6-140-112-31-65.jp.ngrok.io/silverlinkapp/data?step={step_count}&heartrate={heartrate_tosend}&sos={sos_status}&fall={fall_status}'
                r = req.get(url)
                sos_status = 0
                fall_status = 0

            n += 1
           

