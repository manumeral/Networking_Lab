import base64, getpass, socket, ssl, sys
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


# Function - getServerAddr
# Description - asks the user to input the mail server's address
def getServAddr():
    global mailserv,flag,i
    local = mailfrom
    i = 0
    for i in range(len(mailfrom)-1):
        if mailfrom[i] == "@":
            break
    i = i+1
    sub = mailfrom[i:]
    if sub == "gmail.com" :
        mailserv = "smtp.gmail.com"
    elif sub == "yahoo.com" :
        mailserv = "smtp.mail.yahoo.com"
    elif sub == "live.com" :
        mailserv = "smtp.live.com"
        flag = 2
    else :
        flag = 1

def getServPort():
    global mailport,flag
    p = int(input('Enter the port number to connect to: '))
    while(p>65535 or p<0):
        print('Invalid entry. Port number must be between 0 and 65,535.')
        p = int(input('Enter the port number to connect to: '))
    mailport = p

def getFromAddr():
    global mailfrom
    mailfrom = input('From: ')
    global username
    username = mailfrom

def getRcptAddr():
    global mailrcpt,cc,bcc
    mailrcpt = input('To: ')
    cc = input('CC(enter the ids seperated with a ;): ').split(';')
    bcc = input('BCC(enter the ids seperated with a ;): ').split(';')

def getMailMess():
    global mailmess,subject
    print('SUBJECT :')
    subject = input()
    print('Mail Body:')
    mailmess = sys.stdin.read(-1)

def getPassword():
    global password
    password = getpass.getpass('Enter your password: ')

def getCryptoOpt():
    global cryptmethod
    cryptmethod = 'SSL'

def dispMenu():
    getFromAddr()
    getServAddr()
    getServPort()
    getRcptAddr()
    getPassword()
    print("Crypto:SSL")
    getCryptoOpt()

# Function - mainLoop
# Description - handles the main loop of the program
def mainLoop():
    useropt = 'N'
    while useropt != 'Y':
        dispMenu()
        useropt = input('Enter (Y/N) to send: ')
        
        if useropt == 'Y':
            getMailMess()
            smtpSession()
        else:
            print('Invalid choice. Please enter again.')

# Function - getSSLSocket
# Description - creates a new socket, wraps it in an SSL context, and returns it
def getSSLSocket():
    return ssl.wrap_socket(socket.socket(socket.AF_INET, socket.SOCK_STREAM), ssl_version=ssl.PROTOCOL_SSLv23)

# Function - smtpSession
# Description - handles sending the message
def smtpSession():
    # Get the socket
    sock = getSSLSocket()
    sock.connect((mailserv, mailport))
    # Receive response from server and print it
    respon = sock.recv(2048)
    print(str(respon, 'utf-8'))
    # Say HELO and print response
    heloMesg = 'HELO Mayank\r\n'
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
    for itr in range(l):
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
#help from https://gist.github.com/Longlius/3735084
