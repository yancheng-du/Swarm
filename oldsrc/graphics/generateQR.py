from __future__ import print_function
import datetime
import os
import pickle
import qrcode
import os.path
import pyqrcode
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
from apiclient.http import MediaFileUpload

cwd = os.getcwd()
SCOPES = ['https://www.googleapis.com/auth/drive']

uploadURI = 'POST https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable';
metadataURI = 'POST https://www.googleapis.com/drive/v3/files';
drive_service = None

def createFolder():
    currentDate = generateFileName(False)
    query = "mimeType = 'application/vnd.google-apps.folder' and name='" + currentDate + "'"
    results = drive_service.files().list(q=query).execute()
    if(results.get('files', [])):
        files = results.get('files')

        print('Folder already exists for %s: %s' % (currentDate,  files[0]['id']))
        return files[0]['id']
    else:
        file_metadata = {
            'name': currentDate,
            'mimeType': 'application/vnd.google-apps.folder',
            'viewersCanCopyContent': True
        }
        file = drive_service.files().create(body=file_metadata,
                                            fields='id').execute()
        print('Folder ID: %s' % file.get('id'))
        return file['id']

def generateFileName(long):
    now = datetime.datetime.now()
    if(long):
        return now.strftime("%Y_%m_%d_%H_%M_%S")
    return now.strftime("%Y_%m_%d")

creds = None
if os.path.exists('token.pickle'):
    with open('token.pickle', 'rb') as token:
        creds = pickle.load(token)
if not creds or not creds.valid:
    if creds and creds.expired and creds.refresh_token:
        creds.refresh(Request())
    else:
        flow = InstalledAppFlow.from_client_secrets_file(
            cwd + '/graphics/credentials.json', SCOPES)
        creds = flow.run_local_server(port=0)
            # Save the credentials for the next run
    with open('token.pickle', 'wb') as token:
        pickle.dump(creds, token)

drive_service = build('drive', 'v3', credentials=creds)
currentDate = generateFileName(False)
query = "mimeType = 'application/vnd.google-apps.folder' and name='" + currentDate + "'"
parentFolder = createFolder()
vidName = generateFileName(True) + ".mp4"

fileMetadata = {
    'name': vidName,
    'parents': [parentFolder],
    'mimeType': 'video/mp4'
}
media = MediaFileUpload('./test.mp4', mimetype='video/mp4', resumable=True)
file = drive_service.files().create(body=fileMetadata,
                                media_body=media,
                                fields='id').execute()
new_permission = {
  'type': 'anyone',
  'role': 'reader'
}
print('Video uploaded: %s' % file.get('id'))
drive_service.permissions().create(fileId=file.get('id'), body=new_permission).execute()

url = "https://drive.google.com/file/d/" + file.get('id') + "/view?usp=sharing"
print(url)
big_code = pyqrcode.create(str(url), error='L', mode='binary')
big_code.png(cwd + '/graphics/video.png', module_color=[0, 0, 0, 128], background=[0xff, 0xff, 0xcc])
