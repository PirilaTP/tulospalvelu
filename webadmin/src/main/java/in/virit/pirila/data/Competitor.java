package in.virit.pirila.data;

public class Competitor {
    private Long id;
    private String competitionNumber;
    private String name;
    private String club;
    private String cardNumber;

    public Competitor() {
    }

    public Competitor(Long id, String competitionNumber, String name, String club, String cardNumber) {
        this.id = id;
        this.competitionNumber = competitionNumber;
        this.name = name;
        this.club = club;
        this.cardNumber = cardNumber;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getCompetitionNumber() {
        return competitionNumber;
    }

    public void setCompetitionNumber(String competitionNumber) {
        this.competitionNumber = competitionNumber;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getClub() {
        return club;
    }

    public void setClub(String club) {
        this.club = club;
    }

    public String getCardNumber() {
        return cardNumber;
    }

    public void setCardNumber(String cardNumber) {
        this.cardNumber = cardNumber;
    }
}
