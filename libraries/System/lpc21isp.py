import RPi.GPIO as GPIO
from time import sleep
from subprocess import run
from sys import argv

def main():
    print("Making sure Putty isn't running")
    print(["killall","putty"])
    run(["killall","putty"])
    reset_board=16 # Reset pin connected to pin 16
    bootstrap_board=18 # Bootstrap connected to pin 18
    # Use board numbering. The library knows what internal (like P0.xx)
    # number to use with each pin, so this has better compatibility.
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup([reset_board,bootstrap_board],GPIO.OUT)
    GPIO.output([reset_board,bootstrap_board],1)
    
    print("Set the bootstrap pin to low (activate the active-low signal)")
    GPIO.output(bootstrap_board,0)
    sleep(2.1)
    print("Set the reset pin to low (activate the active-low signal)")
    GPIO.output(reset_board,0)
    sleep(2.1)
    print("Set the reset pin back to high, to release from reset")
    GPIO.output(reset_board,1)
    sleep(2.1)
    print("Wait a moment, and set bootstrap pin back to high")
    GPIO.output(bootstrap_board,1)
    sleep(2.1)
    print("Run the programmer")
    print(["lpc21isp"]+argv[1:])
    run(["lpc21isp"]+argv[1:])
    print("Reset the board again, after programming")
    GPIO.output(reset_board,0)
    sleep(2.1)
    print("Release from reset")
    GPIO.output(reset_board,1)
    sleep(2.1)
    print("Release the GPIO device")
    GPIO.cleanup()
    


if __name__=="__main__":
    main()


