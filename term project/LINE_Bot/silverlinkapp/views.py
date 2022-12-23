from django.shortcuts import render

# Create your views here.

from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseForbidden
from django.views.decorators.csrf import csrf_exempt
from django.conf import settings
 
from linebot import LineBotApi, WebhookParser
from linebot.exceptions import InvalidSignatureError, LineBotApiError
from linebot.models import MessageEvent, TextSendMessage
 
line_bot_api = LineBotApi(settings.LINE_CHANNEL_ACCESS_TOKEN)
parser = WebhookParser(settings.LINE_CHANNEL_SECRET)
 
step = 0
heartrate = 0
sos = 0
fall = 0
flag = 0
 
@csrf_exempt
def callback(request):
    global step, heartrate, flag
    if request.method == 'POST':
        signature = request.META['HTTP_X_LINE_SIGNATURE']
        body = request.body.decode('utf-8')
 
        try:
            events = parser.parse(body, signature)  # 傳入的事件
        except InvalidSignatureError:
            return HttpResponseForbidden()
        except LineBotApiError:
            return HttpResponseBadRequest()
 
        for event in events:
            if isinstance(event, MessageEvent):  # 如果有訊息事件
                if '步數' in event.message.text:
                    line_bot_api.reply_message(  # 回復傳入的訊息文字
                        event.reply_token,
                        TextSendMessage(text=f'今日累積步數為{step}步')
                    )
                if '心率' in event.message.text:
                    line_bot_api.reply_message(  # 回復傳入的訊息文字
                        event.reply_token,
                        TextSendMessage(text=f'現在心跳為每秒{heartrate}下')
                    )
                if '收到' in event.message.text:
                    line_bot_api.reply_message(  # 回復傳入的訊息文字
                        event.reply_token,
                        TextSendMessage(text=f'將繼續發送心率異常通知')
                    )
                    flag = 0

        return HttpResponse()
    else:
        return HttpResponseBadRequest()

@csrf_exempt
def data(request):
    global step, heartrate, flag
    if request.method == 'GET':
        step = int(request.GET['step'])
        heartrate = int(request.GET['heartrate'])
        sos = int(request.GET['sos'])
        fall = int(request.GET['fall'])
        if sos:
            line_bot_api.broadcast(TextSendMessage(text='緊急情況!!\n緊急按鈕已被按下'))
        if fall:
            line_bot_api.broadcast(TextSendMessage(text='偵測到跌到!!'))
        if heartrate < 60 and flag == 0:
            line_bot_api.broadcast(TextSendMessage(text=f'心率過低\n現在心跳為每秒{heartrate}下'))
            flag = 1
        if heartrate > 100 and flag == 0:
            line_bot_api.broadcast(TextSendMessage(text=f'心率過高\n現在心跳為每秒{heartrate}下'))
            flag = 1
        return HttpResponse()
    else:
        return HttpResponseBadRequest()