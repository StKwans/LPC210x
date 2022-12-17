import RPi.GPIO as GPIO
import argparse


def main():
    reset_board=16 # Reset pin connected to pin 16
    bootstrap_board=18 # Bootstrap connected to pin 18
    # Use board numbering. The library knows what internal (like P0.xx)
    # number to use with each pin, so this has better compatibility.
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup([reset_board,bootstrap_board],GPIO.OUT)
    GPIO.output([reset_board,bootstrap_board],1)
    parser=argparse.ArgumentParser(description='Control the LPC210x chip on the Raspberry Pi Rollercoasterometer hat. '
                                               'This controls the reset (board pin 16) and bootstrap (board pin 18) ' 
                                               'pins. Note that these pins are active low -- ie activating one or '
                                               'the other will set low voltage, and deactivating it will set high voltage')
    parser.add_argument('-r','--reset-activate',action='store_true',help='Activate (set low) the reset pin')
    parser.add_argument('-R','--reset-deactivate',action='store_true',help='Deactivate (set high) the reset pin')
    parser.add_argument('-b','--bootstrap-activate',action='store_true',help='Activate (set low) the bootstrap pin')
    parser.add_argument('-B','--bootstrap-deactivate',action='store_true',help='Deactivate (set high) the bootstrap pin')
    parser.add_argument('-q','--cleanup',action='store_true',help='Release GPIO driver')

    args=parser.parse_args()
    if args.reset_activate:
        GPIO.output(reset_board,0)
    if args.reset_deactivate:
        GPIO.output(reset_board,1)
    if args.bootstrap_activate:
        GPIO.output(bootstrap_board,0)
    if args.bootstrap_deactivate:
        GPIO.output(bootstrap_board,1)
    if args.cleanup:
        GPIO.cleanup()


if __name__=="__main__":
    main()


