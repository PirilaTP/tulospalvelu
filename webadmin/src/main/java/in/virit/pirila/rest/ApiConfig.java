package in.virit.pirila.rest;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.web.servlet.FilterRegistrationBean;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.Ordered;

/**
 * Wires the {@link ApiKeyAuthFilter} so it guards only the REST endpoints
 * under {@code /api/*}. The filter is instantiated here (rather than being a
 * {@code @Component}) so Spring Boot does not also auto-register it for every
 * URL, which would force the Vaadin UI behind the api key as well.
 */
@Configuration
public class ApiConfig {

    @Bean
    public FilterRegistrationBean<ApiKeyAuthFilter> apiKeyAuthFilterRegistration(
            @Value("${tulospalvelu.api.key:}") String apiKey) {
        FilterRegistrationBean<ApiKeyAuthFilter> registration = new FilterRegistrationBean<>();
        registration.setFilter(new ApiKeyAuthFilter(apiKey));
        registration.addUrlPatterns("/api/*");
        registration.setOrder(Ordered.HIGHEST_PRECEDENCE);
        registration.setName("apiKeyAuthFilter");
        return registration;
    }
}
