package in.virit.pirila.service;

import fi.pirila.tulospalvelu.KilpReader;
import in.virit.pirila.data.Competitor;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class CompetitorService {

    private final TulospalveluService tulospalveluService;

    public CompetitorService(TulospalveluService tulospalveluService) {
        this.tulospalveluService = tulospalveluService;
    }

    public List<Competitor> getAllCompetitors() {
        return tulospalveluService.getCompetitors().stream()
                .map(this::toCompetitor)
                .toList();
    }

    public List<Competitor> searchCompetitors(String searchTerm) {
        String term = searchTerm.toLowerCase();
        return tulospalveluService.getCompetitors().stream()
                .filter(c -> String.valueOf(c.kilpno).contains(term)
                        || (c.sukunimi + " " + c.etunimi).toLowerCase().contains(term)
                        || c.seura.toLowerCase().contains(term))
                .map(this::toCompetitor)
                .toList();
    }

    public Competitor getCompetitorById(Long id) {
        KilpReader.Competitor comp = tulospalveluService.getCompetitorByRecordIndex(id.intValue());
        return comp != null ? toCompetitor(comp) : null;
    }

    private Competitor toCompetitor(KilpReader.Competitor comp) {
        return new Competitor(
                (long) comp.recordIndex,
                String.valueOf(comp.kilpno),
                comp.sukunimi + " " + comp.etunimi,
                comp.seura,
                comp.badge > 0 ? String.valueOf(comp.badge) : ""
        );
    }
}
