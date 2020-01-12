import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/** This is a ButtonDemo Application.
 * @version $Id: ButtonDemo1.java,v 1.1.1.1 1999/05/03 21:49:23 ian Exp $
 */
public class ButtonDemo1 extends JFrame implements ActionListener {
	JButton b1;

	/** Construct a ButtonDemo1 */
	public ButtonDemo1() {
			super("Linux Journal Button Demo");
			getContentPane().add(b1 = new JButton("A button"));
			b1.addActionListener(this);
			pack();
	}
 
	public void actionPerformed(ActionEvent event) {
	    System.exit(0);
	}

	public static void main(String av[]) {
	    new ButtonDemo1().setVisible(true);
	}
}
