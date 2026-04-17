package in.virit.pirila.data;

public class Competitor {
    private Long id;
    private String competitionNumber;
    private String name;
    private String club;
    private String cardNumber;
    private String sarja;
    private String result;
    private int resultOrder; // for sorting: ysija for placed, high values for DNF/DNS/open

    public Competitor() {
    }

    public Competitor(Long id, String competitionNumber, String name, String club,
                      String cardNumber, String sarja, String result, int resultOrder) {
        this.id = id;
        this.competitionNumber = competitionNumber;
        this.name = name;
        this.club = club;
        this.cardNumber = cardNumber;
        this.sarja = sarja;
        this.result = result;
        this.resultOrder = resultOrder;
    }

    public Long getId() { return id; }
    public void setId(Long id) { this.id = id; }

    public String getCompetitionNumber() { return competitionNumber; }
    public void setCompetitionNumber(String competitionNumber) { this.competitionNumber = competitionNumber; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getClub() { return club; }
    public void setClub(String club) { this.club = club; }

    public String getCardNumber() { return cardNumber; }
    public void setCardNumber(String cardNumber) { this.cardNumber = cardNumber; }

    public String getSarja() { return sarja; }
    public void setSarja(String sarja) { this.sarja = sarja; }

    public String getResult() { return result; }
    public void setResult(String result) { this.result = result; }

    public int getResultOrder() { return resultOrder; }
    public void setResultOrder(int resultOrder) { this.resultOrder = resultOrder; }
}
