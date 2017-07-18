from time import time,ctime

c = 10

def p():
    print 'p is used'

def test_add(a, b):
    print a,b
    print a+b[0]
    print 'Today is',ctime(time())
    p()
    print c
    return a+b[0]
