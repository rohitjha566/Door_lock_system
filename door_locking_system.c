#include <LPC21xx.h>

#define rs (1 << 16)
#define e (1 << 17)
#define MOTOR_IN1 (1 << 18) // Motor input 1 connected to P0.18
#define MOTOR_IN2 (1 << 19) // Motor input 2 connected to P0.19

// 7 Segment and LCD functions
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 12000; j++);
    }
}

void cmd(char cmd) {
    IOCLR0 = 0xFF;          // Clear all port P0 bits
    IOSET0 = cmd;            // Send command to P0
    IOCLR0 = rs;             // RS = 0 for command
    delay_ms(2);           // Delay to process
    IOSET0 = e;              // Set Enable (E pin)
    delay_ms(2);
    IOCLR0 = e;              // Clear Enable (E pin)
    delay_ms(2);
}

void data(char data) {
    IOCLR0 = 0xFF;          // Clear all port P0 bits
    IOSET0 = data;           // Send data to P0
    IOSET0 = rs;             // RS = 1 for data
    delay_ms(2);           // Delay to process
    IOSET0 = e;              // Set Enable (E pin)
    delay_ms(2);
    IOCLR0 = e;              // Clear Enable (E pin)
    delay_ms(2);
}

void lcd_init() {
    IODIR0 |= 0xFF;          // Set P0.0 to P0.7 as output (for LCD data lines)
    IODIR0 |= (rs | e);      // Set RS and E as output
    delay_ms(10);            // Delay to ensure LCD is powered up
    cmd(0x38);               // 8-bit mode, 2-line, 5x7 font
    cmd(0x01);               // Clear display
    cmd(0x0C);               // Display on, cursor off
    cmd(0x06);               // Entry mode set, increment cursor
}

void print_string(const char *str) {
    while (*str) {
        data(*str++);
    }
}

// Keypad pin connections
#define ROW0 16
#define ROW1 17
#define ROW2 18
#define ROW3 19
#define COL0 20
#define COL1 21
#define COL2 22
#define COL3 23

unsigned char keypad_lut[4][4] = {
    '7', '8', '9', '%',
    '4', '5', '6', '*',
    '1', '2', '3', '-',
    'c', '0', '=', '+'
};

unsigned char row_val, col_val, KeyC;
unsigned int c0, c1, c2, c3;

void keypad_init() {
    IODIR1 |= (1 << ROW0 | 1 << ROW1 | 1 << ROW2 | 1 << ROW3);  // Rows as output
    IODIR1 &= ~(1 << COL0 | 1 << COL1 | 1 << COL2 | 1 << COL3); // Columns as input
}

char keypad_getkey() {
    while (1) {
        IOCLR1 = (1 << ROW0 | 1 << ROW1 | 1 << ROW2 | 1 << ROW3); // Set rows low
        while (1) {
            c0 = (IOPIN1 >> COL0) & 1;
            c1 = (IOPIN1 >> COL1) & 1;
            c2 = (IOPIN1 >> COL2) & 1;
            c3 = (IOPIN1 >> COL3) & 1;
            if ((c0 & c1 & c2 & c3) == 0)
                break;
        }
        
        // Row identification
        while (1) {
            IOCLR1 = 1 << ROW0;  // Set ROW0 low
            IOSET1 = (1 << ROW1 | 1 << ROW2 | 1 << ROW3); // Other rows high
            c0 = (IOPIN1 >> COL0) & 1;
            c1 = (IOPIN1 >> COL1) & 1;
            c2 = (IOPIN1 >> COL2) & 1;
            c3 = (IOPIN1 >> COL3) & 1;
            if ((c0 & c1 & c2 & c3) == 0) {
                row_val = 0;
                break;
            }
            
            IOCLR1 = 1 << ROW1;  // Set ROW1 low
            IOSET1 = (1 << ROW0 | 1 << ROW2 | 1 << ROW3);
            c0 = (IOPIN1 >> COL0) & 1;
            c1 = (IOPIN1 >> COL1) & 1;
            c2 = (IOPIN1 >> COL2) & 1;
            c3 = (IOPIN1 >> COL3) & 1;
            if ((c0 & c1 & c2 & c3) == 0) {
                row_val = 1;
                break;
            }
            
            IOCLR1 = 1 << ROW2;  // Set ROW2 low
            IOSET1 = (1 << ROW0 | 1 << ROW1 | 1 << ROW3);
            c0 = (IOPIN1 >> COL0) & 1;
            c1 = (IOPIN1 >> COL1) & 1;
            c2 = (IOPIN1 >> COL2) & 1;
            c3 = (IOPIN1 >> COL3) & 1;
            if ((c0 & c1 & c2 & c3) == 0) {
                row_val = 2;
                break;
            }
            
            IOCLR1 = 1 << ROW3;  // Set ROW3 low
            IOSET1 = (1 << ROW0 | 1 << ROW1 | 1 << ROW2);
            c0 = (IOPIN1 >> COL0) & 1;
            c1 = (IOPIN1 >> COL1) & 1;
            c2 = (IOPIN1 >> COL2) & 1;
            c3 = (IOPIN1 >> COL3) & 1;
            if ((c0 & c1 & c2 & c3) == 0) {
                row_val = 3;
                break;
            }
        }
        
        // Column identification
        if (((IOPIN1 >> COL0) & 1) == 0) col_val = 0;
        else if (((IOPIN1 >> COL1) & 1) == 0) col_val = 1;
        else if (((IOPIN1 >> COL2) & 1) == 0) col_val = 2;
        else if (((IOPIN1 >> COL3) & 1) == 0) col_val = 3;
        
        while (1) {
            c0 = (IOPIN1 >> COL0) & 1;
            c1 = (IOPIN1 >> COL1) & 1;
            c2 = (IOPIN1 >> COL2) & 1;
            c3 = (IOPIN1 >> COL3) & 1;
            if ((c0 & c1 & c2 & c3) != 0)
                break;
        }
        
        return keypad_lut[row_val][col_val];
    }
}

int compare_passwords(const unsigned char *password, const unsigned char *input) {
    unsigned int i;
    for (i = 0; i < 4; i++) {
        if (password[i] != input[i]) {
            return 0; // Passwords do not match
        }
    }
    return 1; // Passwords match
}

void forward() {
    IOSET0 = MOTOR_IN1;
    IOCLR0 = MOTOR_IN2;
}

void reverse() {
    IOSET0 = MOTOR_IN2;
    IOCLR0 = MOTOR_IN1;
}

void stop() {
    IOCLR0 = MOTOR_IN1 | MOTOR_IN2;
}

int main() {
    unsigned char pass[4] = "1234"; // Set your Password
    unsigned char input[4];
    int pos = 0;
    unsigned char key;

    IO0DIR |= MOTOR_IN1 | MOTOR_IN2; // Set motor control pins as output

    lcd_init();
    keypad_init();

    while (1) {
        cmd(0x80);
			print_string("PWD:");

        pos = 0;
        while (pos < 4) {
            key = keypad_getkey();
            if (key != 'c' && key != '=') {
                data(key);
                input[pos++] = key;
            } else if (key == 'c' && pos > 0) {
                pos--;
                data(' '); // Erase last character on LCD
            } else if (key == '=') {
                if (compare_passwords(pass, input)) {
                    cmd(0x80);
                    print_string("Password OK");
                    forward(); // Motor forward
                    delay_ms(2000); // Motor run duration
                    stop(); // Stop motor
                    delay_ms(1000); // Delay before next attempt
                    break;
                } else {
                    cmd(0x80);
                    print_string("Wrong Password");
                    delay_ms(1000);
                }
                pos = 0; // Reset input
                cmd(0x01); // Clear display
            }
        }
    }
}
