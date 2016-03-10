#!/usr/bin/python

# Name: Matthew Longley
# Date: 2012/09/25
# Course: COMP 3825-001
# Project #1 - Mail Client

# NOTE: This program uses Python 3

import base64, getpass, socket, ssl, sys

# Global variables
mailserv = ''
mailport = -1
mailfrom = ''
mailrcpt = ''
mailmess = ''
username = ''
password = ''
cryptmethod = 'none'
msg = ''
subject = ''
cc = []
bcc = []
flag = 0

# Derpy hooves ascii art for maximum swag
derpyAscii = """Sourav"""


# Function - getServerAddr
# Description - asks the user to input the mail server's address
def getServAddr():
    global mailserv,flag,i
    local = mailfrom
    i = 0
    #mailserv = input('Enter the address of the mail server: ')
    for i in range(len(mailfrom)-1):
        if mailfrom[i] == "@":
            break
    #mailfrom.find('@')
    i = i+1
    #print(i)
    sub = mailfrom[i:]
    #print(sub)
    if sub == "gmail.com" :
        mailserv = "smtp.gmail.com"
    elif sub == "yahoo.com" :
        mailserv = "smtp.mail.yahoo.com"
    elif sub == "live.com" :
        mailserv = "smtp.live.com"
        flag = 2
    else :
        flag = 1
        #print('Cannot send mail other than Gmail or Yahoo mail ')


# Function - getServerPort
# Description - asks the user to input the mail server's port number
def getServPort():
    global mailport,flag
    #p = int(input('Enter the port number to connect to: '))
    if flag == 2 :
        p = 587
    else:
        p = 465
    if not (p < 0 or p > 65535):
        mailport = p
    else:
        print('Invalid entry. Port number must be between 0 and 65,535.')
    
            

# Function - getFromAddr
# Description - asks the user to input the email address they're sending from
def getFromAddr():
    global mailfrom
    mailfrom = input('From: ')

# Function - getRcptAddr
# Description - asks the user to input the email address they're sending to
def getRcptAddr():
    global mailrcpt,cc,bcc
    mailrcpt = input('To: ')
    #print('CC(enter the ids seperated with a ;): ')
    cc = input('CC(enter the ids seperated with a ;): ').split(';')
    #print (cc[:])
    bcc = input('BCC(enter the ids seperated with a ;): ').split(';')

# Function - getMailMsg
# Description - asks the user to input a message terminated with an EOF to send
def getMailMess():
    global mailmess,subject
    print('SUBJECT :')
    subject = input()

    print('--------------------------------------------------------------------------------')
    print('|                              Mail Body                                       |')
    print('--------------------------------------------------------------------------------')
#    mailmess = sys.stdin.read(-1)

# Function - getUserName
# Description - asks the user to input their username to authenticate
def getUserName():
    global username
    username = mailfrom

# Function - getPassword
# Description - asks the user to input their password to authenticate
def getPassword():
    global password
    password = getpass.getpass('Enter your password: ')

# Function - getCryptoOpt
# Description - asks the user to select their method of encryption
def getCryptoOpt():
    global cryptmethod
    while True:
        #c = input('Choose an encryption protocol (TLS, SSL, or none): ')
        c = 'SSL'
        if (c == 'TLS') or (c == 'SSL') or (c == 'none'):
            cryptmethod = c
            return
        else:
            print("Invalid choice!")
        

# Function - dispMenu
# Description - displays the program's main menu
def dispMenu():

    
    getFromAddr()

    getServAddr()

    if mailport == -1:
        getServPort()
    else:
        getServPort()
    
    
    getRcptAddr()

    getUserName()

    
    getPassword()
    #print("7) Crypto: " + cryptmethod)
    getCryptoOpt()

# Function - mainLoop
# Description - handles the main loop of the program
def mainLoop():
    useropt = 'derp'
    while useropt != 'Y':
        dispMenu()
        useropt = input('Enter (Y/n) to send: ')
        
        if useropt == 'Y':
            getMailMess()
            smtpSession()
        else:
            print('Invalid choice. Please enter again.')

# Function - getSSLSocket
# Description - creates a new socket, wraps it in an SSL context, and returns it
def getSSLSocket():
    return ssl.wrap_socket(socket.socket(socket.AF_INET, socket.SOCK_STREAM), ssl_version=ssl.PROTOCOL_SSLv23)

# Function - getTLSSocket
# Description - creates a new socket, wraps it in a TLS context, and returns it
#def getTLSSocket():
    return ssl.wrap_socket(socket.socket(socket.AF_INET, socket.SOCK_STREAM), ssl_version=ssl.PROTOCOL_TLSv1)

# Function - getPlainSocket
# Description - creates a new vanilla socket and returns it
#def getPlainSocket():
    return socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Function - smtpSession
# Description - handles sending the message
def smtpSession():
    # Get the socket
    if cryptmethod == 'SSL':
        sock = getSSLSocket()
    elif cryptmethod == 'TLS':
        sock = getTLSSocket()
    else:
        sock = getPlainSocket()
    # Attempt to connect to the SMTP server
    sock.connect((mailserv, mailport))
    # Receive response from server and print it
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    # Say HELO and print response
    heloMesg = 'HELO Sourav\r\n'
    print(heloMesg)
    sock.send(heloMesg.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    # Authenticate with the server
    authMesg = 'AUTH LOGIN\r\n'
    crlfMesg = '\r\n'
    print(authMesg)
    sock.send(authMesg.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    user64 = base64.b64encode(username.encode('utf-8'))
    pass64 = base64.b64encode(password.encode('utf-8'))
    print('user64 = ' + str(user64))
    sock.send(user64)
    sock.send(crlfMesg.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    print('pass64 = ' + str(pass64))
    sock.send(pass64)
    sock.send(crlfMesg.encode('utf-8'))
    respon = sock.recv(2048)
    print('respon : ' + str(respon, 'utf-8'))
    # Tell server the message's sender
    fromMesg = 'MAIL FROM: <' + mailfrom + '>\r\n'
    print(fromMesg)
    sock.send(fromMesg.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    # Tell server the message's recipient
    total = []
    total = cc + bcc
    total.insert(0,mailrcpt)
    l = len(total)
    for itr in range(l-1):
        rcptMesg = 'RCPT TO: <' + total[itr] +'>\r\n'
        print(rcptMesg)
        sock.send(rcptMesg.encode('utf-8'))
        respon = sock.recv(2048)
        print(str(respon, 'utf-8'))
    
    # Give server the message
    dataMesg = 'DATA\r\n'
    print(dataMesg)
    sock.send(dataMesg.encode('utf-8'))
    
    submesg = 'SUBJECT :' + subject + '\r\n'
    sock.send(submesg.encode('utf-8'))
    tomsg = 'TO :' + mailrcpt + '\r\n'
    sock.send(tomsg.encode('utf-8'))
    for itr in cc:
        ccmsg = 'CC :' + itr + '\r\n'
        sock.send(ccmsg.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    mailbody = mailmess +  '\r\n'
    #mailbody = """From:""" + mailfrom[0:i-1] + """  <""" + mailfrom + """>
    #To: To Person <""" + mailrcpt + """>
    #Cc:""" + str(cc[:]) + """
    #Subject:""" + subject + """
    #""" + mailmess +  '\r\n'

#    message = "From: %s\r\n" % mailfrom
#    + "To: %s\r\n" % mailrcpt
#    + "CC: %s\r\n" % ",".join(cc)
#    + "Subject: %s\r\n" % subject
#    + "\r\n" 
#    + mailmess + '\r\n'
    print(mailbody)
    sock.send(mailbody.encode('utf-8'))
    fullStop = '\r\n.\r\n'
    print(fullStop)
    sock.send(fullStop.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    # Signal the server to quit
    quitMesg = 'QUIT\r\n'
    print(quitMesg)
    sock.send(quitMesg.encode('utf-8'))
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    # Close the socket to finish
    sock.close()

mainLoop()
