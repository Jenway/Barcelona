NAME=inception
DATA_DIR = data

SERVICES = mariadb wordpress

DATA_SUBDIRS = $(addprefix $(DATA_DIR)/, $(SERVICES))

prepare:
	mkdir -p $(DATA_SUBDIRS)

up: prepare
	docker compose -f srcs/docker-compose.yml --env-file srcs/.env up -d --build

down:
	docker compose -f srcs/docker-compose.yml --env-file srcs/.env down

clean:
	docker rmi -f nginx-custom wordpress-custom mariadb-custom || true
	docker volume rm $(docker volume ls -qf "name=$(USER)") || true

fclean:
	docker compose -f srcs/docker-compose.yml --env-file srcs/.env down --volumes --remove-orphans
	docker rmi -f nginx-custom wordpress-custom mariadb-custom || true
	-docker volume rm wordpress_web_data wordpress_db_data || true
	-docker network rm inception || true

log:
	docker compose -f srcs/docker-compose.yml --env-file srcs/.env logs

re: fclean up

