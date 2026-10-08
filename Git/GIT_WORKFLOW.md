# Git workflow recomendado para Rosetta

Ruta usada actualmente:
`D:\\Desarrollo\\STM32\\Projects\\Rosetta\\Rosetta_Base`

## 1. Congelar el bring-up estable

Abre la terminal integrada de VS Code dentro de `Rosetta_Base`:

```powershell
git init
git branch -M main
```

Copia el `.gitignore` de esta carpeta `Git` a la raíz del proyecto como
`.gitignore`, después:

```powershell
git add .
git commit -m "Rosetta A.0 stable bring-up"
git tag -a bringup-v0.1.1 -m "Stable Rosetta A.0 bring-up"
```

El tag es la fotografía congelada. `main` puede seguir avanzando después.

## 2. Crear Acceptance Test

```powershell
git switch -c acceptance-test
```

Ahora aplica los archivos del Acceptance Test y trabaja normalmente. Cuando una
etapa quede estable:

```powershell
git add .
git commit -m "Add Rosetta acceptance test pages and buzzer"
```

Para probar el LSE no bloqueante haz otro commit separado, no mezcles ambos
cambios en el mismo commit.

## 3. Volver al bring-up en cualquier momento

Sin modificar nada:

```powershell
git switch main
```

Para inspeccionar exactamente la versión congelada:

```powershell
git switch --detach bringup-v0.1.1
```

Después vuelve con:

```powershell
git switch acceptance-test
```

## 4. Rama para la demo de la canción

Cuando Acceptance ya compile y quieras jugar con la OLED:

```powershell
git switch main
git switch -c demo-wake-me-up
```

Si quieres que la demo herede también los drivers nuevos del Acceptance Test,
créala desde esa rama en vez de `main`:

```powershell
git switch acceptance-test
git switch -c demo-wake-me-up
```

## 5. Ver dónde estás

```powershell
git status
git branch
git log --oneline --decorate --graph --all
```

No uses `git reset --hard` mientras estés aprendiendo Git salvo que tengas claro
qué cambios vas a descartar.
