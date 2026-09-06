# Politique de Sécurité et d'Accès

## Vue d'ensemble

Ce dépôt contient une implémentation d'un décodeur LDPC quantique avec support FPGA pour la carte AMD RFSoC ZCU111. Les artifacts matériels (bitstream, images bootloader, configurations de synthèse) contiennent des designs propriétaires et sont traités comme des ressources sensibles.

## Contrôle d'accès aux artifacts

### Artifacts sensibles

Les fichiers suivants générés par le workflow CI matériel sont **restreints** :

- `download.bit` - Bitstream FPGA (configuration hardware)
- `BOOT.BIN` - Bootloader PetaLinux
- `image.ub` - Image Linux compressée
- `*.xsa` - Export Xilinx pour réutilisation

### Politique d'accès

1. **Branche `main`** :
   - Merges protégés : exige au minimum une revue approuvée
   - Déploiement matériel manuel uniquement : `workflow_dispatch` avec `run_hardware=true`
   - Exécution sur runner auto-hébergé sécurisé (`self-hosted`, `vivado`, `zcu111`)

2. **Pull Requests** :
   - **Ne déclenchent JAMAIS** le job matériel (`hardware-bitstream-build`)
   - Prévention des attaques par injection via code externe
   - Seule la CI logicielle s'exécute (`ubuntu-latest`)

3. **Workflow_dispatch (Manuel)** :
   - Réservé aux opérateurs de confiance
   - Génère les artifacts sensibles
   - Conservés 30 jours maximum
   - Accessibles uniquement aux collaborateurs authentifiés

### Permissions GitHub Actions

```yaml
permissions:
  contents: read        # Lecture seule du code
  actions: read         # Lecture des exécutions (artifacts restreints)
```

Les artifacts sensibles ne peuvent être téléchargés que par :
- Le propriétaire du dépôt (`owner`)
- Les collaborateurs avec accès `push` ou `admin`
- Les utilisateurs authentifiés sur le dépôt privé (si applicable)

## Protections de branche

La branche `main` doit être protégée avec :

- ✅ Require pull request reviews before merging
- ✅ Require branches to be up to date before merging
- ✅ Require status checks to pass before merging (especially `software-ci`)
- ✅ Dismiss stale pull request approvals when new commits are pushed
- ✅ Require approval of the latest reviewable commit

## Déploiement matériel

### Processus sécurisé

```bash
# 1. Fusionner les changements dans main via PR revue
# 2. Attendre que software-ci passe
# 3. Déclencher manuellement le workflow :
#    - Aller à GitHub Actions > QLDPC Software and FPGA CI
#    - Cliquer "Run workflow"
#    - Cocher "Run HLS, Vivado, PetaLinux, and HIL checks..."
#    - Confirmer
# 4. Surveiller l'exécution sur le runner ZCU111 de confiance
# 5. Télécharger les artifacts (accès restreint)
```

## Bonnes pratiques de sécurité

1. **Ne jamais** utiliser un runner auto-hébergé sans protection
2. **Ne jamais** exécuter le workflow matériel depuis une PR externe
3. **Inspecter** toute modification des fichiers CMake et Makefile
4. **Conserver** les credentials Xilinx en dehors du dépôt (envar sur runner local)
5. **Auditer** les accès aux artifacts via les logs GitHub

## Incidents de sécurité

Pour signaler une vulnérabilité :

1. **Ne pas** créer d'issue publique
2. Contacter le propriétaire du dépôt via mail privé
3. Fournir : description, impact, recommandation de correction
4. Attendre la confirmation avant toute divulgation

## Mise à conformité

Dernier audit : 2026-09-06

Checklist :
- [x] Workflow matériel protégé (workflow_dispatch uniquement)
- [x] Pull requests sans accès matériel
- [x] Artifacts sensibles restreints à 30 jours
- [x] Runner auto-hébergé en confiance configuré
- [x] Permissions minimales appliquées
- [ ] Branche main protégée (à faire via GitHub UI)
- [ ] Audit des accès périodique
