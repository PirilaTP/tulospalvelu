package in.virit.pirila.emit;

import org.springframework.stereotype.Component;

import java.util.function.Consumer;

@Component
public class EmitCardReader {

    private Consumer<String> cardNumberCallback;

    public void startReading(Consumer<String> callback) {
        this.cardNumberCallback = callback;
        System.out.println("Emit card reading started (mock)");
    }

    public void stopReading() {
        this.cardNumberCallback = null;
        System.out.println("Emit card reading stopped (mock)");
    }

    public void simulateCardRead(String cardNumber) {
        System.out.println("Simulated card read: " + cardNumber);
        if (cardNumberCallback != null) {
            cardNumberCallback.accept(cardNumber);
        }
    }
}
