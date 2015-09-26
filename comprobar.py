import MySQLdb                                                                                                                                                                       
import sys                                                                                                                                                                           
                                                                                                                                                                                     
                                                                                                                                                                                     
host = "127.0.0.1"                                                                                                                                                                   
user = "your-user"                                                
passw = "password"                                                                                                                                                               
base = "arduino"                                                                                                                                                                     
                                                                                                                                                                                     
while True:                                                                                                                                                                          
                                                                                                                                                                                     
        db= MySQLdb.connect(host,user,passw,base)                                                                                                                                    
        cur = db.cursor()                                                                                                                                                            
        resultado = cur.execute("""SELECT * FROM usuariosrfid WHERE rfid LIKE %s ORDER BY id""",(sys.argv[1],))                                                                      
        if (resultado == 1 ):                                                                                                                                                        
                print 1                                                                                                                                                              
                sys.exit(1)                                                                                                                                                          
        else:                                                                                                                                                                        
                print 2                                                                                                                                                              
                sys.exit(1)   
