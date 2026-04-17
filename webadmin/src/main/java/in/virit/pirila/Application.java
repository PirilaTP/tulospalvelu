package in.virit.pirila;

import com.vaadin.open.Open;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * The entry point of the Spring Boot application.
 */
@SpringBootApplication
public class Application {

    public static void main(String[] args) {
        var ctx = SpringApplication.run(Application.class, args);
        var port = ctx.getEnvironment().getProperty("local.server.port", "8080");
        Open.open("http://localhost:" + port);
    }

}
