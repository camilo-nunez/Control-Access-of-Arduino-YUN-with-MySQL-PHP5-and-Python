import MySQLdb                                               
import sys                                                   
import time                                                  
                                                             
host = "127.0.0.1"                                           
user = "your-user"                                                
passw = "password"                                       
base = "arduino"                                             
                                                             
                                                             
tiempo = time.strftime("%d/%m/%Y %H:%M:%S", time.localtime())
db= MySQLdb.connect(host,user,passw,base)
cur = db.cursor()    
resultado = cur.execute(""" INSERT INTO ControlUsuarios (rfid,fecha) VALUES (%s,%s) """,(sys.argv[1],tiempo,))
if (resultado == 1 ):
        print 1    
        sys.exit(1)        
else:                
        print 2                
        sys.exit(1) 
