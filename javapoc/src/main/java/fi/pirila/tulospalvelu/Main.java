package fi.pirila.tulospalvelu;

/**
 * Main entry point for the Tulospalvelu Java POC - Emit Card Changer.
 * Delegates to InteractiveEmitChanger which auto-reads configuration
 * from kisat/HkMaaliData/laskenta.cfg and competitors from KILP.DAT.
 */
public class Main {

    public static void main(String[] args) throws Exception {
        InteractiveEmitChanger.main(args);
    }
}
