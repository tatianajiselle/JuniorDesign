library ieee;
use ieee.std_logic_arith.all;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity JuniorDesign is
	port(
		-- LCD Display
		Set_LCD_Data                   : in  std_logic_vector (7 downto 0); -- goes to PIN_P18 -> PIN_K21
		Set_LCD_EN, Set_LCD_RS         : in  std_logic; -- goes to PIN_J22 & PIN_E25, respectively
		LCD_Data                       : out std_logic_vector (7 downto 0); -- goes to LCD_D[7] -> LCD_D[0]
		LCD_EN, LCD_RS, LCD_ON, LCD_RW : out std_logic);
end entity;

architecture HardwareSettings of JuniorDesign is
begin	
	-- Set up LCD Display
	LCD_Data <= Set_LCD_Data;
	LCD_EN <= Set_LCD_EN;
	LCD_RS <= Set_LCD_RS;
	LCD_ON <= '1';
	LCD_RW <= '0';
end architecture;