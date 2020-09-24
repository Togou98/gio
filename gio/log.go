package gio

import (
	"fmt"
	"log"
	"os"
	"time"
)


type logLevel int
var logCh chan logBody
const (
	CONSOLEVEL logLevel = iota
	DEBUGLEVEL
	INFOLEVEL
	WARNLEVEL
	ERRORLEVEL
)
type logBody struct {
	message string
	level logLevel
}
func SetLogLevel(level logLevel) {
	level = level
}
var level logLevel
var logfile *os.File
func init() {
	go initLog()
}
func logRoutineStart(){
	if logfile != nil{
		for {
			select{
			case msg := <- logCh :{
				if msg.level == CONSOLEVEL{
					fmt.Print(msg.message)
				}else{
					log.Print(msg.message)
				}
			}
			}
		}
	}
}
func initLog(){
	var err error
	for {
		logfile, err = os.OpenFile("gio"+ time.Now().Format("0102")+".log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, os.ModePerm)
		if err != nil {
			panic("gio.log File Init Error")
		}
		log.SetFlags(log.Ldate)
		//log.SetOutput(logfile)
		log.SetPrefix("<Gio> ")
		logCh = make(chan logBody,0xFF)
		go logRoutineStart()
		time.Sleep(24 * time.Hour)
		logfile.Close()
		close(logCh)
	}
}
const(
	consoleCyan = "\033[1;36m<%s>\033[0m%s"
	//debugCyan  ="\\033[1;36m[DEBUG]\\033[0m"
	//infoGreen = "\\033[1;36m[DEBUG]\\033[0m"
	//warnPink = "\\033[1;35m[WARN]\\033[0m"
	//errorRed = "\\033[0;31m[ERROR]\\033[0m"
)
func now2str(f string)string{
	return time.Now().Format(f)
}
func ConSole(str string){
	if level <= CONSOLEVEL{
		logCh<-logBody{fmt.Sprintf(consoleCyan,now2str("03:04:06"),str),CONSOLEVEL}
	}
}
func DEBUG(str string) {
	if level <= DEBUGLEVEL {
		logCh<-logBody{fmt.Sprintf("[DEBUG]" + str),DEBUGLEVEL}
	}
}
func INFO(str string) {
	if level <= INFOLEVEL {
		log.Printf("[INFO]" + str)
		logCh<-logBody{fmt.Sprintf("[INFO]" + str),INFOLEVEL}
	}
}
func WARN(str string) {
	if level <= WARNLEVEL {
		log.Printf("[WARN]" + str)
		logCh<-logBody{fmt.Sprintf("[WARN]" + str),WARNLEVEL}
	}
}
func ERROR(str string) {
	if level <= ERRORLEVEL {
		logCh<-logBody{fmt.Sprintf("[ERROR]" + str),ERRORLEVEL}
	}
}
