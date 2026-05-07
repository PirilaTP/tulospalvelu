package in.virit.pirila.data;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;

/**
 * Form-bindable copy of a competitor for the CompetitorListView edit panel.
 * recordIndex is the immutable KILP.DAT slot (== Competitor.id internally);
 * etunimi/sukunimi/seura/sarja go into a KILPT message; cardNumber goes into
 * a KILPPVT message via the same multi-stage logic as the standalone card-
 * change view.
 */
public class CompetitorEdit {

    @NotNull
    private Integer recordIndex;

    /** Cleared display field, not sent. Just shown so the form has the runner's bib. */
    private String kilpno;

    @NotBlank
    @Size(max = 24, message = "Korkeintaan 24 merkkiä")
    private String etunimi;

    @NotBlank
    @Size(max = 24, message = "Korkeintaan 24 merkkiä")
    private String sukunimi;

    @Size(max = 31, message = "Korkeintaan 31 merkkiä")
    private String seura;

    /**
     * Club abbreviation (seuralyh in KILP.DAT). Auto-populated when the user
     * picks a known seura from the catalogue; null/blank for custom entries.
     */
    @Size(max = 15, message = "Korkeintaan 15 merkkiä")
    private String seuralyh;

    /** District/area code (piiri INT16). Auto-populated alongside seuralyh; 0 = unset. */
    private Integer piiri;

    @NotNull
    private Integer sarja;

    /** New badge or null if unchanged. Allow empty (no badge) too. */
    @Pattern(regexp = "^$|^[0-9]{1,8}$", message = "Numeromuoto, korkeintaan 8 numeroa")
    private String cardNumber;

    /**
     * pv[0].tlahto as 24h "HH:MM[:SS]" or empty for "not set".
     * Parsed in CompetitorListView.handleSave; unparseable input is rejected
     * by the @Pattern below.
     */
    @Pattern(regexp = "^$|^([01]\\d|2[0-3]):[0-5]\\d(:[0-5]\\d)?$",
            message = "Anna 24h aika muodossa HH:MM tai HH:MM:SS")
    private String startTime;

    public CompetitorEdit() {}

    public Integer getRecordIndex() { return recordIndex; }
    public void setRecordIndex(Integer recordIndex) { this.recordIndex = recordIndex; }

    public String getKilpno() { return kilpno; }
    public void setKilpno(String kilpno) { this.kilpno = kilpno; }

    public String getEtunimi() { return etunimi; }
    public void setEtunimi(String etunimi) { this.etunimi = etunimi; }

    public String getSukunimi() { return sukunimi; }
    public void setSukunimi(String sukunimi) { this.sukunimi = sukunimi; }

    public String getSeura() { return seura; }
    public void setSeura(String seura) { this.seura = seura; }

    public String getSeuralyh() { return seuralyh; }
    public void setSeuralyh(String seuralyh) { this.seuralyh = seuralyh; }

    public Integer getPiiri() { return piiri; }
    public void setPiiri(Integer piiri) { this.piiri = piiri; }

    public Integer getSarja() { return sarja; }
    public void setSarja(Integer sarja) { this.sarja = sarja; }

    public String getCardNumber() { return cardNumber; }
    public void setCardNumber(String cardNumber) { this.cardNumber = cardNumber; }

    public String getStartTime() { return startTime; }
    public void setStartTime(String startTime) { this.startTime = startTime; }
}
